#include "common.h"
#include "InGameServer.h"
#include "LoginServer.h"
#include "Client.h"
#include "Player.h"
#include "RigidBody.h"

InGameServer::InGameServer()
	: mLoginPtr{ nullptr }
{
	mGameConstants = std::make_shared<GameConstant>();

	mBtCarShape = std::make_unique<BtCarShape>("Resource\\Car_Data.bin", "Resource\\Models\\Car_Body_Convex_Hull.obj");
	mMissileShape = std::make_unique<BtMissileShape>("Resource\\Models\\Missile_Convex_Hull.obj");
	mMapShapes[0] = std::make_unique<BtMapShape>("Resource\\MapData.tmap");
	mMapShapes[1] = std::make_unique<BtMapShape>("Resource\\MapData_night.tmap");
	mCheckpointShape = std::make_unique<CheckpointShape>("Resource\\CheckPoint.tmap");
}

void InGameServer::Init(LoginServer* loginPtr, RoomList& roomList)
{
	mLoginPtr = loginPtr;
	mTimerQueue.Start(this);

	for (int i = 0; i < MAX_ROOM_SIZE; i++)
	{
		msWorlds[i] = std::make_unique<GameWorld>(mGameConstants);
		msWorlds[i]->InitPhysics(-9.8f);	
		msWorlds[i]->InitPlayerList(roomList[i].get(), (int)mCheckpointShape->GetInfos().size());
	}
}

void InGameServer::PrepareToStartGame(int roomID, char mapIdx)
{
	btVector3 offset = { 0.0f,0.0f,0.0f };
	const auto& players = msWorlds[roomID]->GetPlayerList();
	for (int i = 0; i < players.size(); i++)
	{
		if (players[i]->Empty) continue;

		const int hostID = players[i]->ID;
		if (gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::IN_GAME) == false)
		{
			mLoginPtr->Disconnect(hostID);
			continue;
		}

		btVector3 offset = mOffset;
		offset.setX(offset.x() * i);
		if (i % 2 == 1)
		{
			offset.setZ(-offset.z());
		}

		msWorlds[roomID]->SetPlayerTransform(i,
			mGameConstants->StartPosition + offset,
			mGameConstants->StartRotation);

		msWorlds[roomID]->CreateRigidbodies(i, 1000.0f, *mBtCarShape, 1.0f, *mMissileShape);
	}
	
	msWorlds[roomID]->InitMapRigidBody(*mMapShapes[mapIdx].get(), *mCheckpointShape.get());
	msWorlds[roomID]->SendGameStartSuccess();
}

bool InGameServer::ProcessPacket(std::byte* packet, const CS::PCK_TYPE& type, int id, int bytes)
{
	switch (type)
	{
	case CS::PCK_TYPE::LOAD_DONE:
	{
		CS::packet_load_done* pck = reinterpret_cast<CS::packet_load_done*>(packet);
		if (pck->room_id < 0 || pck->room_id != gClients[id]->RoomID)
		{
			mLoginPtr->Disconnect(id);
			break;
		}

		bool res = msWorlds[pck->room_id]->CheckIfAllLoaded(gClients[id]->PlayerIndex);
		if (res) StartMatch(pck->room_id);
		break;
	}
	case CS::PCK_TYPE::KEY_INPUT:
	{
		CS::packet_key_input* pck = reinterpret_cast<CS::packet_key_input*>(packet);
		
		const int idx = (int)gClients[id]->PlayerIndex;
		const int roomID = gClients[id]->RoomID;
		if (roomID < 0 || idx < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		msWorlds[roomID]->HandleKeyInput(idx, pck->key, pck->pressed);
		break;
	}
	default:
		std::cout << "Invalid packet.\n";
		return false;
	}
	return true;
}

void InGameServer::StartMatch(int roomID)
{
	msWorlds[roomID]->SetActive(true);
	msWorlds[roomID]->SetGameTime(
		mGameConstants->CountdownTime,
		mGameConstants->GameRunningTime);
	msWorlds[roomID]->SendReadySignal();
	AddTimerEvent(roomID, EVENT_TYPE::PHYSICS, mPhysicsDurationMs);
}

void InGameServer::RemovePlayer(int roomID, int hostID)
{
	const int idx = gClients[hostID]->PlayerIndex;
	msWorlds[roomID]->RemovePlayerRigidBody(idx);
}

void InGameServer::AddTimerEvent(int roomID, EVENT_TYPE type, int duration)
{
	TimerQueue::TimerEvent ev{ 
		std::chrono::milliseconds(duration), type, roomID };
	mTimerQueue.AddTimerEvent(ev);
}

void InGameServer::RunPhysicsSimulation(int roomID)
{
#ifdef DEBUG_PACKET_TRANSFER
		//std::cout << "[Room id: " << roomID << "] Running physics simulation.\n";
#endif
	msWorlds[roomID]->UpdatePhysicsWorld();

	if (msWorlds[roomID]->IsActive())
	{
		AddTimerEvent(roomID, EVENT_TYPE::PHYSICS, mPhysicsDurationMs);
	}
	else
	{
		msWorlds[roomID]->FlushPhysicsWorld();
	}
}

void InGameServer::PostPhysicsOperation(int roomID)
{
	IOCP& iocp = mLoginPtr->GetIOCP();
	iocp.PostToCompletionQueue(msWorlds[roomID]->GetPhysicsOverlapped(), roomID);
}