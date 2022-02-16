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
	mVehicleConstants = std::make_shared<VehicleConstant>();

	mBtCarShape = std::make_unique<BtCarShape>("Resource\\Car_Data.bin");
	mTerrainShapes[0] = std::make_unique<BtTerrainShape>("Resource\\PlaneMap_Data.bin");
}

void InGameServer::Init(LoginServer* loginPtr, RoomList& roomList)
{
	mLoginPtr = loginPtr;
	mTimer.Start(this);

	const float offset_x = 20.0f;
	for (int i = 0; i < MAX_ROOM_SIZE; i++)
	{
		msWorlds[i] = std::make_unique<GameWorld>(mVehicleConstants);
		msWorlds[i]->InitPhysics(-10.0f);
		msWorlds[i]->InitPlayerList(mStartPosition, offset_x, roomList[i].get());		
	}
}

void InGameServer::PrepareToStartGame(int roomID)
{
	for (int idx = 0; const auto& player : msWorlds[roomID]->GetPlayerList())
	{
		const int hostID = player->ID;
		if (hostID < 0) continue;

		if (gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::IN_GAME) == false)
		{
			mLoginPtr->Disconnect(hostID);
			continue;
		}

		msWorlds[roomID]->CreatePlayerRigidBody(idx++, 1000.0f, mBtCarShape.get());
	}
	msWorlds[roomID]->InitMapRigidBody(mTerrainShapes[0].get(), mObjRigidBodies);

	// TODO: This needs to be separated.
	AddPhysicsTimerEvent(roomID);
	msWorlds[roomID]->SetActive(true);
	msWorlds[roomID]->SendGameStartSuccess();
}

bool InGameServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
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
		msWorlds[roomID]->HandleKeyInput(idx, pck->key, pck->pressed);
		break;
	}
	default:
		std::cout << "Invalid packet.\n";
		return false;
	}
	return true;
}

void InGameServer::RemovePlayer(int roomID, int hostID)
{
	const int idx = gClients[hostID]->PlayerIndex;
	msWorlds[roomID]->RemovePlayerRigidBody(idx);
}

void InGameServer::AddPhysicsTimerEvent(int roomID)
{
	Timer::TimerEvent ev(
		std::chrono::milliseconds(mDurationMs),
		EVENT_TYPE::PHYSICS, roomID, mDurationMs / 1000.0f);

	mTimer.AddTimerEvent(ev);
}

void InGameServer::RunPhysicsSimulation(int roomID, float timeStep)
{
#ifdef DEBUG_PACKET_TRANSFER
		//std::cout << "[Room id: " << roomID << "] Running physics simulation.\n";
#endif
	msWorlds[roomID]->UpdatePhysicsWorld(timeStep);
	msWorlds[roomID]->BroadcastAllTransform();	// broadcast instantly.

	if (msWorlds[roomID]->IsActive())
		AddPhysicsTimerEvent(roomID);
	else
		msWorlds[roomID]->FlushPhysicsWorld();
}

void InGameServer::PostPhysicsOperation(int roomID, float timeStep)
{
	IOCP& iocp = mLoginPtr->GetIOCP();
	iocp.PostToCompletionQueue(msWorlds[roomID]->GetOverlapped(timeStep), roomID);
}