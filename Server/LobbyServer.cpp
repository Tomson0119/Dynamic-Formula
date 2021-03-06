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

bool LobbyServer::ProcessPacket(std::byte* packet, const CS::PCK_TYPE& type, int id, int bytes)
{
	switch (type)
	{
	case CS::PCK_TYPE::INQUIRE_ROOM:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received inquire room packet\n";
	#endif
		CS::packet_inquire_room* pck = reinterpret_cast<CS::packet_inquire_room*>(packet);
		gClients[id]->SetPageNum(pck->page_num);
		SendExistingRoomList(id);
		break;
	}
	case CS::PCK_TYPE::OPEN_ROOM:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received open room packet\n";
	#endif
		if (int roomID = TryOpenRoom(id); roomID >= 0)
		{
			AcceptEnterRoom(roomID, id);
			mRoomCount.fetch_add(1);
		}
		break;
	}
	case CS::PCK_TYPE::ENTER_ROOM:
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
	case CS::PCK_TYPE::SWITCH_MAP:
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
	case CS::PCK_TYPE::PRESS_READY:
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
	case CS::PCK_TYPE::REVERT_SCENE:
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

int LobbyServer::TryOpenRoom(int hostID)
{
	if (mRoomCount >= MAX_ROOM_SIZE)
	{
		gClients[hostID]->SendAccessRoomDeny(ROOM_STAT::MAX_ROOM_REACHED);
		return -1;
	}

	if (int idx = FindEmptyRoom(); mRooms[idx]->OpenRoom())
	{
		if (gClients[hostID]->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
		{
			mRooms[idx]->CloseRoom();	// if it fails : logic error
			//mLoginPtr->Disconnect(hostID);
			return -1;
		}
		return (mRooms[idx]->AddPlayer(hostID)) ? idx : -1; // if it fails : logic error
	}
	return -1; // logic error
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
			//mLoginPtr->Disconnect(hostID);
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
	{
		const int roomID = gClients[hostID]->RoomID;
		if (roomID < 0)
		{
			mLoginPtr->Disconnect(hostID);
			break;
		}
		if (logout == false)
		{
			if (roomID < 0 || gClients[hostID]->ChangeState(currentState, CLIENT_STAT::IN_ROOM) == false)
			{
				mLoginPtr->Disconnect(hostID);
				break;
			}
		}
		
		mInGameServer.RemovePlayer(roomID, hostID);
		if (mInGameServer.IsWorldActive(roomID) == false
			&& mRooms[roomID]->GameRunning())
		{
			mRooms[roomID]->RevertRoomState();
		}

		if (logout)
		{
			__fallthrough;
		}
		else break;
	}
	case CLIENT_STAT::IN_ROOM:
	{
		const int roomID = gClients[hostID]->RoomID;
		if (roomID < 0)
		{
			mLoginPtr->Disconnect(hostID);
			break;
		}
		if (logout == false)
		{
			if (roomID < 0 || gClients[hostID]->ChangeState(currentState, CLIENT_STAT::LOBBY) == false)
			{
				mLoginPtr->Disconnect(hostID);
				break;
			}
		}

		mRooms[roomID]->RemovePlayer(hostID);
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
			mRooms[roomID]->SetRoomState(ROOM_STAT::GAME_STARTED);

			// update game started flag of room
			SendRoomInfoToLobbyPlayers(roomID);

			// Hand over this room to in game server
			mInGameServer.PrepareToStartGame(roomID, mRooms[roomID]->GetMapIndex());
		}
		else mRooms[roomID]->SendGameStartFail();
	}
	else mRooms[roomID]->ToggleReady(hostID);
}

void LobbyServer::SendRoomInfoToLobbyPlayers(int roomID, int ignore, bool instSend)
{
	int pageNumOfRoomId = FindPageNumOfRoom(roomID);
	if (pageNumOfRoomId < 0)
	{
		std::cout << "Couldn't find page num.\n";
		return;
	}

	int lobbyPlayers = mLobbyPlayerCount;
	for (int i = 0; i < MAX_PLAYER_SIZE && lobbyPlayers > 0; i++)
	{
		if (gClients[i]->GetCurrentState() == CLIENT_STAT::LOBBY 
			&& gClients[i]->GetPageNum() == pageNumOfRoomId
			&& i != ignore)
		{
			SendExistingRoomList(i, instSend);
			lobbyPlayers -= 1;
		}
	}
}

void LobbyServer::SendExistingRoomList(int id, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Sending room outside info packet.\n";
#endif
	SC::packet_room_outside_info pck{};
	pck.size = sizeof(SC::packet_room_outside_info);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::ROOM_OUTSIDE_INFO);

	int startCnt = gClients[id]->GetPageNum() * ROOM_NUM_PER_PAGE + 1;
	int endCnt = startCnt + ROOM_NUM_PER_PAGE;

	int openRoomCnt = 0, k = 0;
	for (int i = 0; i < MAX_ROOM_SIZE && k < ROOM_NUM_PER_PAGE; i++)
	{
		if (mRooms[i]->Closed() == false) openRoomCnt += 1;
		if (startCnt <= openRoomCnt && openRoomCnt <= endCnt)
		{
			if (mRooms[i]->Closed() == false)
			{
				pck.rooms[k].room_id = mRooms[i]->GetID();
				pck.rooms[k].player_count = mRooms[i]->GetPlayerCount();
				pck.rooms[k].game_started = mRooms[i]->GameRunning();
				pck.rooms[k].map_id = mRooms[i]->GetMapIndex();
				pck.rooms[k].room_opened = !mRooms[i]->Closed();
				k += 1;
			}
		}
		else if (openRoomCnt > endCnt) break;
	}
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[id]->SendMsg();
}

int LobbyServer::FindPageNumOfRoom(int roomId)
{
	int cnt = 0;
	for (int i = 0; i < MAX_ROOM_SIZE; i++)
	{
		if (roomId == i) return cnt / ROOM_NUM_PER_PAGE;
		if (mRooms[i]->Closed()) continue;
		cnt += 1;
	}
	return -1;
}

int LobbyServer::FindEmptyRoom()
{
	for (int i = 0; i < mRooms.size(); i++)
	{
		if (mRooms[i]->Closed())
			return i;
	}
	return -1;
}
