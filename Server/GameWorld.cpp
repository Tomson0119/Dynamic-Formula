#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "RigidBody.h"
#include "Timer.h"


GameWorld::GameWorld(std::shared_ptr<InGameServer::VehicleConstant> constantPtr)
	: mID{ -1 }, mActive{ false },
	  mPlayerCount{ 0 },
	  mPhysicsOverlapped{ OP::PHYSICS }
{
	for (int i = 0; i < mPlayerList.size(); i++)
		mPlayerList[i] = nullptr;
	mConstantPtr = constantPtr;
}

void GameWorld::InitPhysics(float gravity)
{
	mPhysics.Init(gravity);
}

void GameWorld::InitMapRigidBody(BtTerrainShape* terrainShape, const std::vector<std::unique_ptr<BtBoxShape>>& objShapes)
{
	mMapRigidBody.CreateTerrainRigidBody(terrainShape);
	// TODO: CreateStaticObjectRigidBodies;
}

void GameWorld::InitPlayerList(const btVector3 startPosition, btScalar offsetX, WaitRoom* room)
{
	mID = room->GetID();

	for (int i = 0; auto& player : mPlayerList)
	{
		player = room->GetPlayerPtr(i);
		player->SetVehicleConstant(mConstantPtr);
		player->SetPosition(
			startPosition.x() + offsetX * i,
			startPosition.y(),
			startPosition.z());
		i++;
	}
}

void GameWorld::CreatePlayerRigidBody(int idx, btScalar mass, BtCarShape* shape)
{
	mPlayerList[idx]->CreateVehicleRigidBody(mass, mPhysics.GetDynamicsWorld(), shape);
	mPlayerCount += 1;
}

void GameWorld::UpdatePhysicsWorld()
{
	mTimer.Tick();
	float elapsed = mTimer.GetElapsed();

	for (Player* player : GetPlayerList())
	{
		if(player->Empty == false)
			player->UpdatePlayerRigidBody(elapsed, mPhysics.GetDynamicsWorld());
	}
	mMapRigidBody.UpdateAllRigidBody(elapsed, mPhysics.GetDynamicsWorld());

	mPhysics.StepSimulation(elapsed);
	
	for (Player* player : GetPlayerList())
	{
		if (player->Empty == false)
			player->UpdateTransformVectors();
	}
}

void GameWorld::FlushPhysicsWorld()
{
	for (Player* player : GetPlayerList())
	{
		player->RemoveRigidBody(mPhysics.GetDynamicsWorld());
	}
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
		mPlayerList[idx]->ToggleKeyValue(key, pressed);
		break;

	case 'Z': // boost
		break;

	case 'X': // missile.
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
		const btVector3 pos = mPlayerList[i]->GetPosition();
		info_pck.x[i] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
		info_pck.y[i] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
		info_pck.z[i] = (int)(pos.z() * FIXED_FLOAT_LIMIT);
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

void GameWorld::PushTransformPacket(int target, int receiver)
{
	SC::packet_player_transform pck{};
	pck.size = sizeof(SC::packet_player_transform);
	pck.type = SC::PLAYER_TRANSFORM;
	pck.player_idx = target;

	const btVector3& pos = mPlayerList[target]->GetPosition();
	const btVector4& quat = mPlayerList[target]->GetQuaternion();
	/*const btVector3& vel = mPlayerList[target]->GetVelocity();
	const btVector3& accel = mPlayerList[target]->GetAcceleration();*/

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	/*pck.velocity[0] = (int)(vel.x() * FIXED_FLOAT_LIMIT);
	pck.velocity[1] = (int)(vel.y() * FIXED_FLOAT_LIMIT);
	pck.velocity[2] = (int)(vel.z() * FIXED_FLOAT_LIMIT);
	
	pck.acceleration[0] = (int)(accel.x() * FIXED_FLOAT_LIMIT);
	pck.acceleration[1] = (int)(accel.y() * FIXED_FLOAT_LIMIT);
	pck.acceleration[2] = (int)(accel.z() * FIXED_FLOAT_LIMIT);*/

	int hostID = mPlayerList[receiver]->ID;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::BroadcastAllTransform()
{
#ifdef DEBUG_PACKET_TRANSFER
	//std::cout << "[room id: " << mID << "] Send all transform packet to all.\n";
#endif

	for (int i = 0; i < mPlayerList.size(); i++)
	{
		if (mPlayerList[i]->Empty) continue;
	
		int id = mPlayerList[i]->ID;
		gClients[id]->SendTransferTime(false);

		for (int j = 0; j < mPlayerList.size(); j++)
		{
			if(mPlayerList[j]->Empty == false)
			{
				PushTransformPacket(i, j);
			}
		}		
		gClients[id]->SendMsg();
	}
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
