#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "RigidBody.h"

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

void GameWorld::UpdatePhysicsWorld(float timeStep)
{
	for (Player* player : GetPlayerList())
	{
		if(player->Empty == false)
			player->UpdatePlayerRigidBody(timeStep, mPhysics.GetDynamicsWorld());
	}
	mMapRigidBody.UpdateAllRigidBody(timeStep, mPhysics.GetDynamicsWorld());

	mPhysics.StepSimulation(timeStep);
	
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
		info_pck.x[i] = pos.x();
		info_pck.y[i] = pos.y();
		info_pck.z[i] = pos.z();
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size);
}

void GameWorld::BroadcastTransform(int idx, int ignore, bool instSend)
{
	if (idx < 0 || idx >= mPlayerList.size()) return;
	if (mPlayerList[idx]->Empty == false)
	{
		SC::packet_player_transform pck{};
		pck.size = sizeof(SC::packet_player_transform);
		pck.type = SC::PLAYER_TRANSFORM;
		pck.player_idx = idx;

		const btVector3& pos = mPlayerList[idx]->GetPosition();
		const btVector3& eul = mPlayerList[idx]->GetEulerAngle();

		pck.position[0] = pos.x();
		pck.position[1] = pos.y();
		pck.position[2] = pos.z();

		pck.euler[0] = eul.x();
		pck.euler[1] = eul.y();
		pck.euler[2] = eul.z();

		SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, ignore, instSend);
	} 
}

void GameWorld::BroadcastAllTransform()
{
#ifdef DEBUG_PACKET_TRANSFER
	//std::cout << "[room id: " << mID << "] Send all transform packet to all.\n";
#endif

	for (int i = 0; i < mPlayerList.size(); i++)
	{
		if (mPlayerList[i]->Empty == false)
		{
			BroadcastTransform(i, -1, false);
		}
	}
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		if (mPlayerList[i]->Empty == false)
		{
			int id = mPlayerList[i]->ID;
			gClients[id]->SendMsg();
		}
	}
}

WSAOVERLAPPEDEX* GameWorld::GetOverlapped(float timeStep)
{
	mPhysicsOverlapped.Reset(OP::PHYSICS, reinterpret_cast<std::byte*>(&timeStep), sizeof(timeStep));
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
