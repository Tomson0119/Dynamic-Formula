#include "common.h"
#include "LoginServer.h"
#include "LobbyServer.h"
#include "InGameServer.h"
#include "Client.h"

InGameServer::WorldList InGameServer::msWorlds;

InGameServer::InGameServer()
{	
}

void InGameServer::Init(LoginServer* loginPtr)
{
	mLoginPtr = loginPtr;
}

void InGameServer::PrepareToStartGame(int roomID, const std::array<int, MAX_ROOM_CAPACITY>& ids)
{
	// TODO: Initialize players..
}

bool InGameServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::KEY_INPUT:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received key input packet.\n";
#endif
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
