#include "common.h"
#include "LoginServer.h"
#include "WaitRoom.h"
#include "Client.h"
#include "LobbyServer.h"

LobbyServer::LobbyServer()
	: mLoginPtr{ nullptr }
{
}

void LobbyServer::Init(LoginServer* ptr)
{
	mLoginPtr = ptr;

	for (int i = 0; i < MAX_ROOM_SIZE; i++)
		mRooms[i] = std::make_unique<WaitRoom>(i);

	mInGameServer.Init(ptr, mRooms);
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
			AcceptEnterRoom(pck->room_id, id);
		else
			gClients[id]->SendAccessRoomDeny(mRooms[pck->room_id]->GetRoomState());

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
		mRooms[roomID]->SwitchMap(id);
		break;
	}
	case CS::PRESS_READY:
	{
		CS::packet_press_ready* pck = reinterpret_cast<CS::packet_press_ready*>(packet);
		
		int roomID = gClients[id]->RoomID;
		if (roomID < 0 || roomID != pck->room_id || gClients[id]->PlayerIndex < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		PressStartOrReady(roomID, id);		
		break;
	}
	case CS::REVERT_SCENE:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received revert scene.\n";
	#endif
		RevertScene(id);
		break;
	}
	default:		
		return mInGameServer.ProcessPacket(packet, type, id, bytes);
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

	if (int roomCount = mRoomCount; mRooms[roomCount]->OpenRoom())
	{
		if (gClients[hostID]->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
		{
			mRooms[roomCount]->CloseRoom();	// if it fails : logic error
			mLoginPtr->Disconnect(hostID);
			return false;
		}
		return mRooms[roomCount]->AddPlayer(hostID); // if it fails : logic error
	}
	return false; // logic error
}

void LobbyServer::AcceptEnterRoom(int roomID, int hostID)
{
	mLobbyPlayerCount.fetch_sub(1);

	gClients[hostID]->SendAccessRoomAccept(roomID, false);
	mRooms[roomID]->SendRoomInsideInfo(hostID);
	mRooms[roomID]->SendUpdatePlayerInfoToAll(hostID, hostID);
	
	SendRoomInfoToLobbyPlayers(roomID);
}

bool LobbyServer::TryEnterRoom(int roomID, int hostID)
{
	if (mRooms[roomID]->Available())
	{
		if (gClients[hostID]->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
		{
			mLoginPtr->Disconnect(hostID);
			return false;
		}
		return mRooms[roomID]->AddPlayer(hostID); // if it fails : logic error.
	}
	return false;
}

void LobbyServer::RevertScene(int hostID, bool logout)
{
	auto currentState = gClients[hostID]->GetCurrentState();
	switch (currentState)
	{
	case CLIENT_STAT::IN_GAME:
	case CLIENT_STAT::IN_ROOM:
	{
		const int roomID = gClients[hostID]->RoomID;

		if (roomID < 0 || gClients[hostID]->PlayerIndex < 0 
			|| gClients[hostID]->ChangeState(currentState, CLIENT_STAT::LOBBY) == false
			|| mRooms[roomID]->RemovePlayer(hostID) == false)
		{
			mLoginPtr->Disconnect(hostID);
			break;
		}

		if(currentState == CLIENT_STAT::IN_GAME)
			mInGameServer.RemovePlayer(roomID, hostID);
		
		// Packet must be sended before initializing client.
		mRooms[roomID]->SendRemovePlayerInfoToAll(hostID);
		SendRoomInfoToLobbyPlayers(roomID, hostID);

		// Clearing information after packet transfer
		gClients[hostID]->RoomID = -1;
		gClients[hostID]->PlayerIndex = -1;

		if (mRooms[roomID]->Closed()) mRoomCount.fetch_sub(1);
		if (logout == false)
		{
			IncreasePlayerCount();
			SendExistingRoomList(hostID);
		}
		break;
	}
	case CLIENT_STAT::LOBBY:
	{
		if (logout == false) 
		{
			DecreasePlayerCount();
			mLoginPtr->Logout(hostID);
		}
		break;
	}
	}
}

void LobbyServer::PressStartOrReady(int roomID, int hostID)
{
	if (mRooms[roomID]->Closed() || mRooms[roomID]->GameRunning())
		return;

	if (mRooms[roomID]->IsAdmin(hostID))
	{
		if (mRooms[roomID]->TryGameStart())
		{
			mRooms[roomID]->ChangeRoomState(ROOM_STAT::AVAILABLE, ROOM_STAT::GAME_STARTED);

			// update game started flag of room
			SendRoomInfoToLobbyPlayers(roomID);

			// Hand over this room to in game server
			mInGameServer.PrepareToStartGame(roomID);
		}
		else mRooms[roomID]->SendGameStartFail();
	}
	else mRooms[roomID]->ToggleReady(hostID);
}

void LobbyServer::SendRoomInfoToLobbyPlayers(int roomID, int ignore, bool instSend)
{
	int lobbyPlayers = mLobbyPlayerCount;
	for (int i = 0; i < MAX_PLAYER_SIZE && lobbyPlayers > 0; i++)
	{
		if (gClients[i]->GetCurrentState() == CLIENT_STAT::LOBBY && i != ignore)
		{
			mRooms[roomID]->SendRoomOutsideInfo(i);
			lobbyPlayers -= 1;
		}
	}
}

void LobbyServer::SendExistingRoomList(int id)
{
	int rooms = mRoomCount;
	for (int i = 0; i < MAX_ROOM_SIZE && rooms > 0; i++)
	{
		if (mRooms[i]->Closed() == false)
		{
			mRooms[i]->SendRoomOutsideInfo(id, false);
			rooms -= 1;
		}
	}
	gClients[id]->SendMsg();
}