#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "RigidBody.h"
#include "Timer.h"
#include "ObjectMask.h"

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

void GameWorld::SetPlayerTransform(int idx, const btVector3& pos, const btQuaternion& quat)
{
	mPlayerList[idx]->SetTransform(pos, quat);
}

void GameWorld::CreateRigidbodies(int idx,
	btScalar carMass, BtCarShape& carShape,
	btScalar missileMass, BtBoxShape& missileShape)
{
	mPlayerList[idx]->CreateVehicleRigidBody(carMass, mPhysics, carShape);
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

		// TEST
		//TestVehicleSpawn();
		// TEST

		UpdatePlayers(elapsed);
		mMap.Update(elapsed, mPhysics);
	}
	mUpdateTick += 1;
	if (mUpdateTick == 2)
	{
		BroadcastAllTransform();
		mUpdateTick = 0;
	}
}

void GameWorld::UpdatePlayers(float elapsed)
{
	for (int i=0; Player* player : GetPlayerList())
	{
		if (player->NeedUpdate())
		{
			player->Update(elapsed, mPhysics);
			if (player->IsInvincible())
			{
				player->UpdateInvincibleDuration(elapsed);
				float duration = player->GetInvincibleDuration();

				if (player->IsActive() == false 
					&& duration <= mConstantPtr->SpawnInterval)
				{
					player->Activate();
					player->StopVehicle();
					SpawnToCheckpoint(*player);					
					SendSpawnPacket(i);
				}
				if (duration <= 0.0f)
				{
					// TODO: need test if client calculation matches servers
					player->ReleaseInvincible();
					player->ChangeVehicleMaskGroup(OBJ_MASK_GROUP::VEHICLE, mPhysics);
				}
			}
		}
		i += 1;
	}
}

void GameWorld::FlushPhysicsWorld()
{
	for (Player* player : GetPlayerList())
	{
		player->Reset(mPhysics);
	}
	mMap.Reset(mPhysics);
	mPhysics.Flush();
}

void GameWorld::RemovePlayerRigidBody(int idx)
{
	mPlayerList[idx]->SetRemoveFlag();
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

	case 'Q': // TEST
	{
		bool b = false;
		mTestFlag.compare_exchange_strong(b, true);
		break;
	}
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
		const btQuaternion& quat = mPlayerList[i]->GetVehicleRigidBody().GetRotation();

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
	const btQuaternion& quat = vehicle.GetRotation();
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
	const btQuaternion& quat = missile.GetRotation();
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

void GameWorld::SendMissileRemovePacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send missile remove packet. [" << target << "]\n";
#endif
	SC::packet_remove_missile pck{};
	pck.size = sizeof(SC::packet_remove_missile);
	pck.type = SC::REMOVE_MISSILE;
	pck.world_id = mID;
	pck.missile_idx = target;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendInvincibleOnPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send invincible on packet. [" << target << "]\n";
#endif
	SC::packet_invincible_on pck{};
	pck.size = sizeof(SC::packet_invincible_on);
	pck.type = SC::INVINCIBLE_ON;
	pck.world_id = mID;
	pck.player_idx = target;
	pck.duration = (int)(mConstantPtr->InvincibleDuration * FIXED_FLOAT_LIMIT);
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendSpawnPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send spawn packet. [" << target << "\n";
#endif
	SC::packet_spawn_transform pck{};
	pck.size = sizeof(SC::packet_spawn_transform);
	pck.type = SC::SPAWN_TRANSFORM;
	pck.world_id = mID;
	pck.player_idx = target;
	
	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetRotation();

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

bool GameWorld::CheckIfAllLoaded(int idx)
{
	if (idx < 0) return false; // logic error
	mPlayerList[idx]->LoadDone = true;

	for (auto& player : mPlayerList)
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

void GameWorld::TestVehicleSpawn()
{
	if (mTestFlag)
	{
		if (mPlayerList[0]->IsInvincible() == false)
		{
			HandleInvincibleMode(0);
		}
		else
		{
			mTestFlag = false;
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
		int aMask = gameObjA.GetMask(objA);
		int bMask = gameObjB.GetMask(objB);

		if (&gameObjB == &mMap)
			HandleCollisionWithMap(idx, mMap.GetCheckpointIndex(objB), aMask);
		else
			HandleCollisionWithPlayer(idx, GetPlayerIndex(gameObjB), aMask, bMask);
	}
}

void GameWorld::HandleCollisionWithMap(int idx, int cpIdx, int mask)
{
	switch (mask)
	{
	case OBJ_MASK::VEHICLE:
	{
		if (cpIdx >= 0)
		{
			auto player = mPlayerList[idx];
			if (player->IsNextCheckpoint(cpIdx))
			{
				player->MarkNextCheckpoint(cpIdx);
				// TODO: may got the point so sort player idx list by point.
				// And send rank of each player to each player.
			}
			else
			{
				int cnt = player->GetReverseDriveCount(cpIdx);
				if (cnt == 1)
				{
					// TODO: Send warning packet to this!! player.
				}
				else if(cnt == 2)
				{
					// player->ResetReverseCount();
					// TODO: Spawn back to last checkpoint.
					// and send spawn packet to all player.
				}
			}
		}
		break;
	}
	case OBJ_MASK::MISSILE:
	{
		if (cpIdx < 0)
		{
			mPlayerList[idx]->DisableMissile();
			SendMissileRemovePacket(idx);
		}
		break;
	}}
}

void GameWorld::HandleCollisionWithPlayer(int aIdx, int bIdx, int aMask, int bMask)
{
	switch (aMask)
	{
	case OBJ_MASK::VEHICLE:
	{
		if (bMask == OBJ_MASK::MISSILE)
		{
			HandleInvincibleMode(aIdx);
		}
		break;
	}
	case OBJ_MASK::MISSILE:
	{
		if (bMask == OBJ_MASK::VEHICLE)
		{
			mPlayerList[aIdx]->IncreasePoint(mConstantPtr->MissileHitPoint);
			mPlayerList[aIdx]->DisableMissile();
			SendMissileRemovePacket(aIdx);
			// TODO: player got point so sort player idx list by point.
			// and send rank of each player to the player
		}
		break;
	}}
}

void GameWorld::HandleInvincibleMode(int idx)
{
	SendInvincibleOnPacket(idx);

	mPlayerList[idx]->SetInvincible();
	mPlayerList[idx]->ClearVehicleComponent();
	mPlayerList[idx]->ChangeVehicleMaskGroup(OBJ_MASK_GROUP::INVINCIBLE, mPhysics);
}

void GameWorld::SpawnToCheckpoint(Player& player)
{
	int currentCPIndex = player.GetCurrentCPIndex();
	if (currentCPIndex < 0) currentCPIndex = mMap.GetCheckpointSize() - 1;
	const RigidBody& cp = mMap.GetCheckpointRigidBody(currentCPIndex);
	player.SetTransform(cp.GetPosition(), cp.GetRotation());
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