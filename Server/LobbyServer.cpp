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

void LobbyServer::AcceptLogin(const char* name, int id)
{
	Client* client = gClients[id].get();
	if (client->ChangeState(CLIENT_STAT::CONNECTED, CLIENT_STAT::LOBBY) == false)
	{
		mLoginPtr->Disconnect(id);
		return;
	}
	mLobbyPlayerCount.fetch_add(1);
	client->Name = name;
	client->SendLoginResult(LOGIN_STAT::ACCEPTED, false);
	SendExistingRoomList(id);
}

void LobbyServer::Logout(int id)
{	
	TryRemovePlayer(gClients[id]->RoomID, id);

	gClients[id]->SetState(CLIENT_STAT::CONNECTED);
	mLobbyPlayerCount.fetch_sub(1);
	//TEST
	//PrintRoomList();
}

bool LobbyServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	Client* client = gClients[id].get();
	switch (type)
	{
	case CS::OPEN_ROOM:
	{
		CS::packet_open_room* pck = reinterpret_cast<CS::packet_open_room*>(packet);
		std::cout << "[" << id << "] Open room packet\n";

		if (mRoomCount < MAX_ROOM_SIZE)
		{
			if (gRooms[mRoomCount]->OpenRoom(id) == false)
			{
				std::cout << "[" << id << " Failed to open room.\n";
				break;
			}
			mLobbyPlayerCount.fetch_sub(1);

			client->SendAccessRoomAccept(mRoomCount, false);
			gRooms[mRoomCount]->SendPlayersInfo(id);
			SendRoomInfoToLobbyPlayers(mRoomCount);

			mRoomCount.fetch_add(1);
		}
		else client->SendAccessRoomDeny(ROOM_STAT::MAX_ROOM_REACHED);
		break;
	}
	case CS::ENTER_ROOM:
	{
		CS::packet_enter_room* pck = reinterpret_cast<CS::packet_enter_room*>(packet);
		std::cout << "[" << id << "] Enter room packet\n";

		if (pck->room_id >= 0)
			gRooms[pck->room_id]->TryAddPlayer(id);
		else
			client->SendAccessRoomDeny(ROOM_STAT::INVALID_ROOM_ID);
		break;
	}
	case CS::REVERT_SCENE:
	{
		auto currentState = client->GetCurrentState();
		if (currentState == CLIENT_STAT::LOBBY)
			Logout(id);
		else if (currentState == CLIENT_STAT::IN_ROOM)
		{
			TryRemovePlayer(client->RoomID, id);
			SendExistingRoomList(id);
		}
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
	//TEST
	//PrintRoomList();
	return true;
}

void LobbyServer::TryRemovePlayer(int roomID, int hostID)
{
	if (roomID >= 0 && gRooms[roomID]->RemovePlayer(hostID))
	{
		if (gRooms[roomID]->Empty()) mRoomCount.fetch_sub(1);

		gClients[hostID]->RoomID = -1;
		SendRoomInfoToLobbyPlayers(roomID);
		mLobbyPlayerCount.fetch_add(1);
		gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::LOBBY);
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
			gRooms[roomID]->SendCurrentRoomInfo(i);
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
			gRooms[i]->SendCurrentRoomInfo(id, false);
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
	for (int i = 0; i < roomCount;)
	{
		if (gRooms.empty() == false)
		{
			std::cout << "---------- " << i + 1 << " ----------\n";
			std::cout << "Room id: " << i << "\n";
			std::cout << "Map index: " << (int)gRooms[i]->GetMapIndex() << "\n";
			std::cout << "Game running: " << std::boolalpha << gRooms[i]->GameRunning() << "\n";
			std::cout << "Player count: " << (int)gRooms[i]->GetPlayerCount() << "\n";
			gRooms[i]->PrintWaitPlayers();
			i += 1;
		}		
	}
	std::cout << "-----------------------\n";
}
