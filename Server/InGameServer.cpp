#include "common.h"
#include "InGameServer.h"
#include "LoginServer.h"
#include "WaitRoom.h"
#include "Client.h"
#include "Player.h"

InGameServer::WorldList InGameServer::msWorlds;

InGameServer::InGameServer()
	: mLoginPtr{ nullptr }
{	
}

void InGameServer::Init(LoginServer* loginPtr, RoomList& roomList)
{
	mLoginPtr = loginPtr;

	const float offset_x = 20.0f;
	for (int i = 0; i < MAX_ROOM_SIZE; i++)
	{
		msWorlds[i] = std::make_unique<GameWorld>();
		msWorlds[i]->Physics.Init(-10.0f);
		msWorlds[i]->MapIdx = roomList[i]->GetMapIndex();

		for (int j = 0; auto& player : msWorlds[i]->PlayerList)
		{
			player = roomList[i]->GetPlayerPtr(j);
			player->SetPosition(
				mStartPosition.x() + offset_x * j,
				mStartPosition.y(),
				mStartPosition.z());
			j++;
		}
	}
}

void InGameServer::PrepareToStartGame(int roomID)
{
	for (const auto& player : msWorlds[roomID]->PlayerList)
	{
		const int hostID = player->ID;
		if (hostID < 0) continue;

		if (gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::IN_GAME) == false)
		{
			mLoginPtr->Disconnect(hostID);
			continue;
		}
	}
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
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		const btVector3 pos = msWorlds[roomID]->PlayerList[i]->GetPosition();
		info_pck.x[i] = pos.x();
		info_pck.y[i] = pos.y();
		info_pck.z[i] = pos.z();
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size, roomID);
}

void InGameServer::SendToAllPlayer(std::byte* pck, int size, int roomID, int ignore, bool instSend)
{
	for (const auto& player : msWorlds[roomID]->PlayerList)
	{
		if (player->Empty == false && player->ID != ignore)
		{
			gClients[player->ID]->PushPacket(pck, size);
			if (instSend) gClients[player->ID]->SendMsg();
		}
	}
}

