#include "common.h"
#include "LoginServer.h"
#include "InGameRoom.h"
#include "Client.h"
#include "LobbyServer.h"

LobbyServer::RoomList LobbyServer::gRooms;

LobbyServer::LobbyServer()
	: mLoginPtr{}
{	
}

LobbyServer::~LobbyServer()
{
}

void LobbyServer::Init(LoginServer* ptr)
{
	mLoginPtr = ptr;
	for (int i = 0; i < MAX_ROOM_SIZE; i++)
		gRooms[i] = std::make_unique<InGameRoom>(i, ptr);
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
			if (gRooms[mRoomCount]->OpenRoom(id))
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
	default:
	{
		int roomID = client->RoomID;
		if (roomID >= 0) 
			return gRooms[roomID]->ProcessPacket(packet, type, id, bytes);
		return false;
	}
	}
	return true;
}

void LobbyServer::AcceptEnterRoom(int roomID, int hostID)
{
	mLobbyPlayerCount.fetch_sub(1);
	gClients[hostID]->SendAccessRoomAccept(roomID, false);
	gRooms[roomID]->SendRoomInsideInfo(hostID);
	gRooms[roomID]->SendUpdatePlayerInfoToAll(hostID, hostID);
	SendRoomInfoToLobbyPlayers(roomID);
}

bool LobbyServer::TryAddPlayer(int roomID, int hostID)
{
	if (roomID < 0)
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::INVALID_ROOM_ID);
		return false;
	}
	if (gRooms[roomID]->Full())
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_FULL);
		return false;
	}
	if (gRooms[roomID]->Empty())
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_CLOSED);
		return false;
	}
	if (gRooms[roomID]->GameRunning())
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::GAME_STARTED);
		return false;
	}
	return gRooms[roomID]->AddPlayer(hostID);
}

void LobbyServer::TryRemovePlayer(int roomID, int hostID)
{
	if (roomID >= 0 && gRooms[roomID]->RemovePlayer(hostID))
	{
		if (gRooms[roomID]->Empty()) mRoomCount.fetch_sub(1);

		SendRoomInfoToLobbyPlayers(roomID);
		gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::LOBBY);
		
		gRooms[roomID]->SendRemovePlayerInfoToAll(hostID);
		
		gClients[hostID]->RoomID = -1;
		gClients[hostID]->PlayerIndex = -1;
		mLobbyPlayerCount.fetch_add(1);
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
			gRooms[roomID]->SendRoomOutsideInfo(i);
			lobbyPlayers -= 1;
		}
	}
}

void LobbyServer::SendExistingRoomList(int id)
{
	int rooms = mRoomCount;
	for (int i = 0; i < MAX_ROOM_SIZE && rooms > 0; i++)
	{
		if (gRooms[i]->Empty() == false)
		{
			gRooms[i]->SendRoomOutsideInfo(id, false);
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
		if (gRooms[i]->Empty() == false)
		{
			std::cout << "---------- " << i + 1 << " ----------\n";
			std::cout << "Room id: " << i << "\n";
			std::cout << "Map index: " << (int)gRooms[i]->GetMapIndex() << "\n";
			std::cout << "Game running: " << std::boolalpha << gRooms[i]->GameRunning() << "\n";
			std::cout << "Player count: " << (int)gRooms[i]->GetPlayerCount() << "\n";
			gRooms[i]->PrintWaitPlayers();
			roomCount -= 1;
		}
	}
	std::cout << "-----------------------\n";
}