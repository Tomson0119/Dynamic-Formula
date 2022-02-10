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
	mBtCarShape = std::make_unique<BtCarShape>("Resource\\Car_Data.bin");
	mTerrainShapes[0] = std::make_unique<BtTerrainShape>("Resource\\PlaneMap_Data.bin");
}

void InGameServer::Init(LoginServer* loginPtr, RoomList& roomList)
{
	mLoginPtr = loginPtr;

	const float offset_x = 20.0f;
	for (int i = 0; i < MAX_ROOM_SIZE; i++)
	{
		msWorlds[i] = std::make_unique<GameWorld>();
		msWorlds[i]->InitPhysics(-10.0f);
		msWorlds[i]->InitMapRigidBody(mTerrainShapes[0].get(), mObjRigidBodies);
		msWorlds[i]->InitPlayerList(mStartPosition, offset_x, roomList);		
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

		// TODO: Create each vehicle rigidbody and add to physic engine.
		msWorlds[roomID]->CreatePlayerRigidBody(idx++, 1000.0f, mBtCarShape.get());
	}	 
	// TODO: Post step simulation operation to iocp.
	SendGameStartSuccess(roomID);
}

bool InGameServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::KEY_INPUT:
	{
		CS::packet_key_input* pck = reinterpret_cast<CS::packet_key_input*>(packet);
		if (pck->room_id < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		HandleKeyInput(id, pck->key, pck->pressed);
		break;
	}
	default:
		std::cout << "Invalid packet.\n";
		return false;
	}
	return true;
}

void InGameServer::HandleKeyInput(int id, uint8_t key, bool pressed)
{
	switch (static_cast<int>(key))
	{
	case VK_LEFT:
		break;

	case VK_RIGHT:
		break;

	case VK_UP:
		break;

	case VK_DOWN:
		break;

	case VK_LSHIFT:
		break;

	case 'Z':
		break;

	case 'X':
		break;

	default:
		std::cout << "Invalid key input.\n";
		break;
	}
}

void InGameServer::SendGameStartSuccess(int roomID)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << roomID << "] Send game start packet.\n";
#endif
	SC::packet_game_start_success info_pck{};
	info_pck.size = sizeof(SC::packet_game_start_success);
	info_pck.type = SC::GAME_START_SUCCESS;
	info_pck.room_id = roomID;

	const auto& players = msWorlds[roomID]->GetPlayerList();
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		const btVector3 pos = players[i]->GetPosition();
		info_pck.x[i] = pos.x();
		info_pck.y[i] = pos.y();
		info_pck.z[i] = pos.z();
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size, roomID);
}

void InGameServer::SendToAllPlayer(std::byte* pck, int size, int roomID, int ignore, bool instSend)
{
	for (const auto& player : msWorlds[roomID]->GetPlayerList())
	{
		if (player->Empty == false && player->ID != ignore)
		{
			gClients[player->ID]->PushPacket(pck, size);
			if (instSend) gClients[player->ID]->SendMsg();
		}
	}
}

