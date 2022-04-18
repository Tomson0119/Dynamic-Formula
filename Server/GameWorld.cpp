#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "RigidBody.h"
#include "Timer.h"

GameWorld::GameWorld(std::shared_ptr<InGameServer::GameConstant> constantPtr)
	: mID{ -1 }, mActive{ false },
	  mPlayerCount{ 0 }, mUpdateTick{ 0 },
	  mPhysicsOverlapped{ OP::PHYSICS }
{
	for (int i = 0; i < mPlayerList.size(); i++)
		mPlayerList[i] = nullptr;
	mConstantPtr = constantPtr;
}

GameWorld::~GameWorld()
{
	mPhysics.Flush();
}

void GameWorld::InitPhysics(float gravity)
{
	mPhysics.Init(gravity);
}

void GameWorld::InitMapRigidBody(const BtMapShape& mapShape, const CheckpointShape& cpShape)
{
	mMap.CreateTrackRigidBody(mapShape.GetCompoundShape());
	mMap.CreateCheckpoints(cpShape.GetCollisionShape(), cpShape.GetInfos());
}

void GameWorld::InitPlayerList(WaitRoom* room, int cpCount)
{
	mID = room->GetID();

	for (int i = 0; auto& player : mPlayerList)
	{
		player = room->GetPlayerPtr(i);
		player->SetGameConstant(mConstantPtr);
		player->SetCheckpointCount(cpCount);
		i++;
	}
}

void GameWorld::SetPlayerPosition(int idx, const btVector3& pos)
{
	mPlayerList[idx]->SetPosition(pos);
}

void GameWorld::SetPlayerRotation(int idx, const btQuaternion& quat)
{
	mPlayerList[idx]->SetRotation(quat);
}

void GameWorld::CreateRigidbodies(int idx,
	btScalar carMass, BtCarShape* carShape,
	btScalar missileMass, BtBoxShape* missileShape)
{
	mPlayerList[idx]->CreateVehicleRigidBody(carMass, mPhysics.GetDynamicsWorld(), carShape);
	mPlayerList[idx]->CreateMissileRigidBody(missileMass, missileShape);
	mPlayerCount += 1;
}

void GameWorld::UpdatePhysicsWorld()
{
	mTimer.Tick();
	float elapsed = mTimer.GetElapsed();

	if (elapsed > 0.0f)
	{
		mPhysics.StepSimulation(elapsed);
		CheckCollision();
		for (Player* player : GetPlayerList())
		{
			if (player->Empty == false)
			{
				player->Update(elapsed, mPhysics.GetDynamicsWorld());
			}
		}
		mMap.Update(elapsed, mPhysics.GetDynamicsWorld());
	}
	mUpdateTick += 1;
	if (mUpdateTick == 2)
	{
		BroadcastAllTransform();
		mUpdateTick = 0;
	}
}

void GameWorld::FlushPhysicsWorld()
{
	for (Player* player : GetPlayerList())
	{
		player->Reset(mPhysics.GetDynamicsWorld());
	}
	mMap.Reset(mPhysics.GetDynamicsWorld());
	mPhysics.Flush();
}

void GameWorld::RemovePlayerRigidBody(int idx)
{
	mPlayerList[idx]->SetDeletionFlag();
	mPlayerCount -= 1;

	if (mPlayerCount == 0)
		SetActive(false);
}

void GameWorld::HandleKeyInput(int idx, uint8_t key, bool pressed)
{
	switch (static_cast<int>(key))
	{
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_LSHIFT:
	case 'Z': // boost
	case 'X': // missile.
		mPlayerList[idx]->ToggleKeyValue(key, pressed);
		break;

	default:
		std::cout << "Invalid key input.\n";
		return;
	}
}

void GameWorld::SendGameStartSuccess()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send game start packet.\n";
#endif
	SC::packet_game_start_success info_pck{};
	info_pck.size = sizeof(SC::packet_game_start_success);
	info_pck.type = SC::GAME_START_SUCCESS;
	info_pck.room_id = mID;

	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		const btVector3& pos = mPlayerList[i]->GetVehicleRigidBody().GetPosition();
		const btQuaternion& quat = mPlayerList[i]->GetVehicleRigidBody().GetQuaternion();

		info_pck.px[i] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
		info_pck.py[i] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
		info_pck.pz[i] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

		info_pck.rx[i] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
		info_pck.ry[i] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
		info_pck.rz[i] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
		info_pck.rw[i] = (int)(quat.w() * FIXED_FLOAT_LIMIT);
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size);
}

void GameWorld::SendStartSignal()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending start signal packet.\n";
#endif

	SC::packet_start_signal pck{};
	pck.size = sizeof(SC::packet_start_signal);
	pck.type = SC::START_SIGNAL;
	pck.world_id = mID;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::BroadcastAllTransform()
{
	for (int receiver = 0; receiver < mPlayerList.size(); receiver++)
	{
		if (mPlayerList[receiver]->Empty) continue;

		for (int target = 0; target < mPlayerList.size(); target++)
		{
			if (mPlayerList[target]->Empty == false)
			{
				PushVehicleTransformPacket(target, receiver);
				PushMissileTransformPacket(target, receiver);
			}
		}

		int id = mPlayerList[receiver]->ID;
		if (id < 0) continue;
		gClients[id]->SendMsg(true);
	}
}

void GameWorld::PushVehicleTransformPacket(int target, int receiver)
{
	SC::packet_player_transform pck{};
	pck.size = sizeof(SC::packet_player_transform);
	pck.type = SC::PLAYER_TRANSFORM;
	pck.player_idx = target;

	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetQuaternion();
	const btVector3& lvel = vehicle.GetLinearVelocity();
	const btVector3& avel = vehicle.GetAngularVelocity();

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	pck.linear_vel[0] = (int)(lvel.x() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[1] = (int)(lvel.y() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[2] = (int)(lvel.z() * FIXED_FLOAT_LIMIT);

	pck.angular_vel[0] = (int)(avel.x() * FIXED_FLOAT_LIMIT);
	pck.angular_vel[1] = (int)(avel.y() * FIXED_FLOAT_LIMIT);
	pck.angular_vel[2] = (int)(avel.z() * FIXED_FLOAT_LIMIT);

	int hostID = mPlayerList[receiver]->ID;
	if (hostID < 0) return;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

void GameWorld::PushMissileTransformPacket(int target, int receiver)
{
	if (mPlayerList[target]->CheckMissileExist() == false) return;

	SC::packet_missile_transform pck{};
	pck.size = sizeof(SC::packet_missile_transform);
	pck.type = SC::MISSILE_TRANSFORM;
	pck.missile_idx = target;

	const auto& missile = mPlayerList[target]->GetMissileRigidBody();
	const btVector3& pos = missile.GetPosition();
	const btQuaternion& quat = missile.GetQuaternion();
	const btVector3& lvel = missile.GetLinearVelocity();

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	pck.linear_vel[0] = (int)(lvel.x() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[1] = (int)(lvel.y() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[2] = (int)(lvel.z() * FIXED_FLOAT_LIMIT);

	int hostID = mPlayerList[receiver]->ID;
	if (hostID < 0) return;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

bool GameWorld::CheckIfAllLoaded(int idx)
{
	if (idx < 0) return false; // logic error
	mPlayerList[idx]->LoadDone = true;

	for (auto player : mPlayerList)
	{
		if (player->Empty == false && player->LoadDone == false)
			return false;
	}
	return true;
}

void GameWorld::SetActive(bool active)
{
	mActive = active;
	if (active)
	{
		mTimer.Start();
	}
}

WSAOVERLAPPEDEX* GameWorld::GetPhysicsOverlapped()
{
	mPhysicsOverlapped.Reset(OP::PHYSICS);
	return &mPhysicsOverlapped;
}

void GameWorld::SendToAllPlayer(std::byte* pck, int size, int ignore, bool instSend)
{
	for (const auto& player : mPlayerList)
	{
		if (player->Empty == false && player->ID != ignore)
		{
			gClients[player->ID]->PushPacket(pck, size);
			if (instSend) gClients[player->ID]->SendMsg();
		}
	}
}

void GameWorld::CheckCollision()
{
	int numManifolds = mPhysics.GetNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = mPhysics.GetPersistentManifold(i);

		if (contactManifold->getNumContacts() <= 0) continue;

		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();

		if (objA == nullptr || objB == nullptr) continue;

		GameObject* gameObjA = reinterpret_cast<GameObject*>(objA->getUserPointer());
		GameObject* gameObjB = reinterpret_cast<GameObject*>(objB->getUserPointer());

		HandleCollision(*objA, *objB, *gameObjA, *gameObjB);
		HandleCollision(*objB, *objA, *gameObjB, *gameObjA);
	}
}

void GameWorld::HandleCollision(const btCollisionObject& objA, const btCollisionObject& objB, GameObject& gameObjA, GameObject& gameObjB)
{
	if (int idx = GetPlayerIndex(gameObjA); idx >= 0)
	{
		auto aTag = gameObjA.GetTag(objA);
		auto bTag = gameObjB.GetTag(objB);

		if (&gameObjB == &mMap)
			HandleCollisionWithMap(idx, mMap.GetCheckpointIndex(objB), aTag);
		else
			HandleCollisionWithPlayer(idx, GetPlayerIndex(gameObjB), aTag, bTag);
	}
}

void GameWorld::HandleCollisionWithMap(int idx, int cpIdx, const GameObject::OBJ_TAG& tag)
{
	switch (tag)
	{
	case GameObject::OBJ_TAG::VEHICLE:
	{
		if (cpIdx >= 0)
		{
			mPlayerList[idx]->HandleCheckpointCollision(cpIdx);
		}
		break;
	}
	case GameObject::OBJ_TAG::MISSILE:
	{
		if (cpIdx < 0)
		{
			mPlayerList[idx]->SetMissileDeletionFlag();
			// Send missile remove packet.
		}
		break;
	}
	default:
		std::cout << "Wrong tag!" << std::endl;
		break;
	}
}

void GameWorld::HandleCollisionWithPlayer(int aIdx, int bIdx, const GameObject::OBJ_TAG aTag, const GameObject::OBJ_TAG bTag)
{
	switch (aTag)
	{
	case GameObject::OBJ_TAG::VEHICLE:
	{
		if (bTag == GameObject::OBJ_TAG::MISSILE)
		{
			if (mPlayerList[aIdx]->IsInvincible() == false)
			{
				mPlayerList[aIdx]->SetInvincible();
				// TODO: Waits for 1.5 second and send spawn packet.
				// and waits another 1.5 second and release invincible mode.
			}
		}
		break;
	}
	case GameObject::OBJ_TAG::MISSILE:
	{
		if (bTag == GameObject::OBJ_TAG::VEHICLE)
		{
			mPlayerList[aIdx]->IncreasePoint(mConstantPtr->MissileHitPoint);
			mPlayerList[aIdx]->SetMissileDeletionFlag();
			// Send missile remove packet.
		}
		break;
	}
	default:
		std::cout << "Wrong tag!" << std::endl;
		break;
	}
}

int GameWorld::GetPlayerIndex(const GameObject& obj)
{
	for (int i=0; const auto& player : mPlayerList)
	{
		if (&obj == player) return i;
		i += 1;
	}
	return -1;
}