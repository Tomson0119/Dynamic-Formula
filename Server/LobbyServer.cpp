#include "common.h"
#include "LoginServer.h"
#include "WaitRoom.h"
#include "Client.h"
#include "LobbyServer.h"

LobbyServer::RoomList LobbyServer::msRooms;

LobbyServer::LobbyServer()
	: mLoginPtr{ nullptr }
{
}

void LobbyServer::Init(LoginServer* ptr)
{
	mLoginPtr = ptr;
	for (int i = 0; i < MAX_ROOM_SIZE; i++)
		msRooms[i] = std::make_unique<WaitRoom>(i, ptr);
}

void LobbyServer::TakeOverNewPlayer(int hostID)
{
	IncreasePlayerCount();
	SendExistingRoomList(hostID);
}

bool LobbyServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	Client* client = gClients[id].get();
	switch (type)
	{
	case CS::OPEN_ROOM:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received open room packet\n";
#endif
		CS::packet_open_room* pck = reinterpret_cast<CS::packet_open_room*>(packet);

		if (mRoomCount < MAX_ROOM_SIZE)
		{
			if (msRooms[mRoomCount]->OpenRoom(id))
			{
				AcceptEnterRoom(mRoomCount, id);
				mRoomCount.fetch_add(1);
			}
			//else std::cout << "Failed to open room\n";
		}
		else client->SendAccessRoomDeny(ROOM_STAT::MAX_ROOM_REACHED);
		break;
	}
	case CS::ENTER_ROOM:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received enter room packet\n";
#endif
		CS::packet_enter_room* pck = reinterpret_cast<CS::packet_enter_room*>(packet);
		
		// for stress test
		gClients[id]->AccessRoomSendTime = pck->send_time;

		if (TryAddPlayer(pck->room_id, id))
			AcceptEnterRoom(pck->room_id, id);
		break;
	}
	case CS::REVERT_SCENE:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received revert scene.\n";
#endif
		auto currentState = gClients[id]->GetCurrentState();
		if (currentState == CLIENT_STAT::LOBBY)
			mLoginPtr->Logout(id);
		else if (currentState == CLIENT_STAT::IN_ROOM)
		{
			RemovePlayer(gClients[id]->RoomID, id);
			SendExistingRoomList(id);
		}
		break;
	}
	default:
	{
		int roomID = client->RoomID;
		if (roomID >= 0) 
			return msRooms[roomID]->ProcessPacket(packet, type, id, bytes);
		return false;
	}
	}
	return true;
}

void LobbyServer::AcceptEnterRoom(int roomID, int hostID)
{
	mLobbyPlayerCount.fetch_sub(1);
	gClients[hostID]->SendAccessRoomAccept(roomID, false);
	msRooms[roomID]->SendRoomInsideInfo(hostID);
	msRooms[roomID]->SendUpdatePlayerInfoToAll(hostID, hostID);
	SendRoomInfoToLobbyPlayers(roomID);
}

bool LobbyServer::TryAddPlayer(int roomID, int hostID)
{
	if (roomID < 0)
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::INVALID_ROOM_ID);
		return false;
	}
	if (msRooms[roomID]->Full())
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_FULL);
		return false;
	}
	if (msRooms[roomID]->Empty())
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_CLOSED);
		return false;
	}
	if (msRooms[roomID]->GameRunning())
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::GAME_STARTED);
		return false;
	}
	return msRooms[roomID]->AddPlayer(hostID);
}

void LobbyServer::RemovePlayer(int roomID, int hostID)
{
	if (roomID >= 0 && msRooms[roomID]->RemovePlayer(hostID))
	{
		if (msRooms[roomID]->Empty()) mRoomCount.fetch_sub(1);

		SendRoomInfoToLobbyPlayers(roomID);
		gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::LOBBY);
		
		msRooms[roomID]->SendRemovePlayerInfoToAll(hostID);
		
		gClients[hostID]->RoomID = -1;
		gClients[hostID]->PlayerIndex = -1;
		DecreasePlayerCount();
	}
}

void LobbyServer::SendRoomInfoToLobbyPlayers(int roomID, bool instSend)
{
	int lobbyPlayers = mLobbyPlayerCount;
	for (int i = 0; i < MAX_PLAYER_SIZE && lobbyPlayers > 0; i++)
	{
		Client* client = gClients[i].get();
		if (client->GetCurrentState() == CLIENT_STAT::LOBBY)
		{
			msRooms[roomID]->SendRoomOutsideInfo(i);
			lobbyPlayers -= 1;
		}
	}
}

void LobbyServer::SendExistingRoomList(int id)
{
	int rooms = mRoomCount;
	for (int i = 0; i < MAX_ROOM_SIZE && rooms > 0; i++)
	{
		if (msRooms[i]->Empty() == false)
		{
			msRooms[i]->SendRoomOutsideInfo(id, false);
			rooms -= 1;
		}
	}
	gClients[id]->SendMsg();
}

void LobbyServer::PrintRoomList()
{
	system("cls");
	std::cout << "Room count: " << mRoomCount << "\n";
	std::cout << "Lobby player count: " << mLobbyPlayerCount << "\n";

	int roomCount = mRoomCount;
	for (int i = 0; i < MAX_ROOM_SIZE && roomCount > 0;i++)
	{
		if (msRooms[i]->Empty() == false)
		{
			std::cout << "---------- " << i + 1 << " ----------\n";
			std::cout << "Room id: " << i << "\n";
			std::cout << "Map index: " << (int)msRooms[i]->GetMapIndex() << "\n";
			std::cout << "Game running: " << std::boolalpha << msRooms[i]->GameRunning() << "\n";
			std::cout << "Player count: " << (int)msRooms[i]->GetPlayerCount() << "\n";
			msRooms[i]->PrintWaitPlayers();
			roomCount -= 1;
		}
	}
	std::cout << "-----------------------\n";
}