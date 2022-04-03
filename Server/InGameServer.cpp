#include "common.h"
#include "InGameServer.h"
#include "LoginServer.h"
#include "WaitRoom.h"
#include "Client.h"
#include "Player.h"
#include "GameWorld.h"
#include "RigidBody.h"

InGameServer::WorldList InGameServer::msWorlds;

InGameServer::InGameServer()
	: mLoginPtr{ nullptr }
{
	mBulletConstants = std::make_shared<BulletConstant>();

	mBtCarShape = std::make_unique<BtCarShape>("Resource\\Car_Data.bin");
	mBtCarShape->LoadConvexHullShape("Resource\\Car_Body_Convex_Hull.obj");
	mMissileShape = std::make_unique<BtBoxShape>("Resource\\Missile_Data.bin");
	mTerrainShapes[0] = std::make_unique<BtTerrainShape>("Resource\\PlaneMap_Data.bin");
}

void InGameServer::Init(LoginServer* loginPtr, RoomList& roomList)
{
	mLoginPtr = loginPtr;
	mTimerQueue.Start(this);

	for (int i = 0; i < MAX_ROOM_SIZE; i++)
	{
		msWorlds[i] = std::make_unique<GameWorld>(mBulletConstants);
		msWorlds[i]->InitPhysics(-10.0f);	
		msWorlds[i]->InitPlayerList(roomList[i].get());
	}
}

void InGameServer::PrepareToStartGame(int roomID)
{
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

		// Test
		if (i == 1)
		{
			btVector3 pos = mStartPosition;
			pos.setZ(pos.z() + 100.0f);

			btQuaternion quat = btQuaternion::getIdentity();
			quat.setRotation({ 0.0f,1.0f,0.0f }, (btScalar)Math::PI);

			msWorlds[roomID]->SetPlayerPosition(i, pos);
			msWorlds[roomID]->SetPlayerRotation(i, quat);
		}
		else
		{
			msWorlds[roomID]->SetPlayerPosition(i, mStartPosition + mOffset * (btScalar)i);
			msWorlds[roomID]->SetPlayerRotation(i, { 0.0f,0.0f,0.0f,1.0f });
		}
		// Test

		msWorlds[roomID]->CreateRigidbodies(i, 1000.0f, mBtCarShape.get(), 1.0f, mMissileShape.get());
	}
	msWorlds[roomID]->InitMapRigidBody(mTerrainShapes[0].get(), mObjRigidBodies);
	msWorlds[roomID]->SendGameStartSuccess();
}

bool InGameServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::TRANSFER_TIME:
	{
		CS::packet_transfer_time* pck = reinterpret_cast<CS::packet_transfer_time*>(packet);
		gClients[id]->SetLatency(pck->send_time);
		break;
	}
	case CS::LOAD_DONE:
	{
		CS::packet_load_done* pck = reinterpret_cast<CS::packet_load_done*>(packet);
		if (pck->room_id < 0 || pck->room_id != gClients[id]->RoomID)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		
		gClients[id]->ReturnSendTimeBack(pck->send_time);

		bool res = msWorlds[pck->room_id]->CheckIfAllLoaded(gClients[id]->PlayerIndex);
		if (res) StartMatch(pck->room_id);
		break;
	}
	case CS::KEY_INPUT:
	{
		CS::packet_key_input* pck = reinterpret_cast<CS::packet_key_input*>(packet);
		
		const int idx = (int)gClients[id]->PlayerIndex;
		const int roomID = gClients[id]->RoomID;
		if (roomID < 0 || idx < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		gClients[id]->ReturnSendTimeBack(pck->send_time);
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
	msWorlds[roomID]->SendStartSignal();
	AddTimerEvent(roomID, EVENT_TYPE::PHYSICS, mPhysicsDuration);
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
		AddTimerEvent(roomID, EVENT_TYPE::PHYSICS, mPhysicsDuration);
	else
		msWorlds[roomID]->FlushPhysicsWorld();
}

void InGameServer::PostPhysicsOperation(int roomID)
{
	IOCP& iocp = mLoginPtr->GetIOCP();
	iocp.PostToCompletionQueue(msWorlds[roomID]->GetPhysicsOverlapped(), roomID);
}