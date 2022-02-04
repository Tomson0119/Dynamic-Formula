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
		if (TryOpenRoom(id))
		{
			AcceptEnterRoom(mRoomCount, id);
			mRoomCount.fetch_add(1);
		}
		break;
	}
	case CS::ENTER_ROOM:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received enter room packet\n";
	#endif
		CS::packet_enter_room* pck = reinterpret_cast<CS::packet_enter_room*>(packet);		
		if (pck->room_id < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}

		if (TryEnterRoom(pck->room_id, id))
			AcceptEnterRoom(mRoomCount, id);
		else 
			gClients[id]->SendAccessRoomDeny(msRooms[pck->room_id]->GetRoomState());

		break;
	}
	case CS::SWITCH_MAP:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received switch map packet.\n";
	#endif
		CS::packet_switch_map* pck = reinterpret_cast<CS::packet_switch_map*>(packet);

		int roomID = gClients[id]->RoomID;
		if (roomID < 0 || roomID != pck->room_id || gClients[id]->PlayerIndex < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		msRooms[roomID]->SwitchMap(id);
		break;
	}
	case CS::PRESS_READY:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received press ready packet.\n";
	#endif
		CS::packet_press_ready* pck = reinterpret_cast<CS::packet_press_ready*>(packet);
		
		int roomID = gClients[id]->RoomID;
		if (roomID < 0 || roomID != pck->room_id || gClients[id]->PlayerIndex < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		msRooms[id]->ToggleReady(id);
		break;
	}
	case CS::REVERT_SCENE:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received revert scene.\n";
	#endif
		if (auto currentState = gClients[id]->GetCurrentState(); currentState == CLIENT_STAT::LOBBY)
			mLoginPtr->Logout(id);
		else if (currentState == CLIENT_STAT::IN_ROOM)
		{
			TryRemovePlayer(gClients[id]->RoomID, id);
			SendExistingRoomList(id);
		}
		break;
	}
	default:		
		return false;
	}
	return true;
}

bool LobbyServer::TryOpenRoom(int hostID)
{
	if (mRoomCount >= MAX_ROOM_SIZE)
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::MAX_ROOM_REACHED);
		return false;
	}

	if (int roomCount = mRoomCount; msRooms[roomCount]->OpenRoom())
	{
		if (gClients[hostID]->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
		{
			msRooms[roomCount]->CloseRoom();	// if it fails : logic error
			mLoginPtr->Disconnect(hostID);
			return false;
		}
		return msRooms[roomCount]->AddPlayer(hostID); // if it fails : logic error
	}
	return false; // logic error
}

void LobbyServer::AcceptEnterRoom(int roomID, int hostID)
{
	mLobbyPlayerCount.fetch_sub(1);
	gClients[hostID]->SendAccessRoomAccept(roomID, false);
	msRooms[roomID]->SendRoomInsideInfo(hostID);
	msRooms[roomID]->SendUpdatePlayerInfoToAll(hostID, hostID);
	SendRoomInfoToLobbyPlayers(roomID);
}

bool LobbyServer::TryEnterRoom(int roomID, int hostID)
{
	if (msRooms[roomID]->Available())
	{
		if (gClients[hostID]->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
		{
			mLoginPtr->Disconnect(hostID);
			return false;
		}
		return msRooms[roomID]->AddPlayer(hostID); // if it fails : logic error.
	}
	return false;
}

bool LobbyServer::TryRemovePlayer(int roomID, int hostID)
{
	if (roomID >= 0 && msRooms[roomID]->RemovePlayer(hostID))
	{
		if (msRooms[roomID]->Closed()) mRoomCount.fetch_sub(1);		
		if (gClients[hostID]->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::LOBBY) == false)
		{
			mLoginPtr->Disconnect(hostID);
			return false;
		}

		gClients[hostID]->RoomID = -1;
		gClients[hostID]->PlayerIndex = -1;
		
		SendRoomInfoToLobbyPlayers(roomID, hostID);
		msRooms[roomID]->SendRemovePlayerInfoToAll(hostID);		

		DecreasePlayerCount();

		return true;
	}
	return false;
}

void LobbyServer::SendRoomInfoToLobbyPlayers(int roomID, int ignore, bool instSend)
{
	int lobbyPlayers = mLobbyPlayerCount;
	for (int i = 0; i < MAX_PLAYER_SIZE && lobbyPlayers > 0; i++)
	{
		if (gClients[i]->GetCurrentState() == CLIENT_STAT::LOBBY && i != ignore)
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
		if (msRooms[i]->Closed() == false)
		{
			msRooms[i]->SendRoomOutsideInfo(id, false);
			rooms -= 1;
		}
	}
	gClients[id]->SendMsg();
}