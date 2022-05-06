#include "common.h"
#include "LoginServer.h"
#include "WaitRoom.h"
#include "Client.h"


WaitRoom::WaitRoom(int id)
	: mID(id), mMapIndex(0), mPlayerCount(0), 
	  mAdminIndex(-1), mState{ ROOM_STAT::ROOM_IS_CLOSED }
{
	for (int i = 0; i < mPlayers.size(); i++)
		mPlayers[i] = std::make_unique<Player>();
}

bool WaitRoom::OpenRoom()
{
	if (Closed())
	{
		// Room can be opened only when room is closed.
		return ChangeRoomState(ROOM_STAT::ROOM_IS_CLOSED, ROOM_STAT::AVAILABLE);
	}
	return false;
}

bool WaitRoom::CloseRoom()
{
	if (Empty())
	{
		mState = ROOM_STAT::ROOM_IS_CLOSED;
		return true;
	}
	return false;
}

int WaitRoom::FindAvaliableSpace()
{
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		if (mPlayers[i]->Empty)
			return i;
	}
	return -1;
}

bool WaitRoom::AddPlayer(int hostID)
{
	const int idx = FindAvaliableSpace();
	if (idx < 0) return false;

	Client* client = gClients[hostID].get();
	client->RoomID = mID;
	client->PlayerIndex = (char)idx;
	if (Empty()) mAdminIndex = idx;

	mPlayers[idx]->ID = hostID;
	mPlayers[idx]->Color = (char)idx;
	mPlayers[idx]->Empty = false;
	mPlayers[idx]->Ready = false;
	strncpy_s(mPlayers[idx]->Name, client->Name.c_str(), MAX_NAME_SIZE - 1);

	IncreasePlayerCount();

	return true;
}

bool WaitRoom::RemovePlayer(int hostID)
{
	if (Empty() == false)
	{
		int idx = (int)gClients[hostID]->PlayerIndex;
		if (idx < 0) return false; // logic error

		mPlayers[idx]->Name[0] = 0;
		mPlayers[idx]->Color = -1;
		mPlayers[idx]->Empty = true;
		mPlayers[idx]->ID = -1;
		mPlayers[idx]->Ready = false;

		DecreasePlayerCount();

		if (idx == mAdminIndex)
		{
			mAdminIndex = [&] {
				for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
					if (mPlayers[i]->Empty == false)
						return i;
				return -1;
			}();
		}
		return true;
	}
	return false;
}

void WaitRoom::SwitchMap(int hostID)
{
	if (gClients[hostID]->PlayerIndex == mAdminIndex)
	{
		mMapIndex = (mMapIndex == 1) ? 0 : 1;
		SendUpdateMapInfoToAll(hostID);
	}
}

bool WaitRoom::TryGameStart()
{
	bool allReady = [&] {
		for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
		{
			if (i == mAdminIndex) continue;
			if (mPlayers[i]->Empty == false && mPlayers[i]->Ready == false)
				return false;
		}
		return (mPlayerCount >= MIN_PLAYER_TO_START);
	}();
	return allReady;
}

void WaitRoom::ToggleReady(int hostID)
{
	const int idx = gClients[hostID]->PlayerIndex;
	mPlayers[idx]->Ready = !mPlayers[idx]->Ready;
	SendUpdatePlayerInfoToAll(hostID, hostID);
}

void WaitRoom::RevertRoomState()
{
	ROOM_STAT expected{ ROOM_STAT::GAME_STARTED };
	ROOM_STAT desired = (mPlayerCount == MAX_ROOM_CAPACITY) 
		? ROOM_STAT::ROOM_IS_FULL : ROOM_STAT::AVAILABLE;
	ChangeRoomState(expected, desired);
}

bool WaitRoom::ChangeRoomState(ROOM_STAT expected, const ROOM_STAT& desired)
{
	return mState.compare_exchange_strong(expected, desired);
}

void WaitRoom::IncreasePlayerCount()
{
	if (mPlayerCount < MAX_ROOM_CAPACITY) 
		mPlayerCount += 1;

	if (mPlayerCount >= MAX_ROOM_CAPACITY)
	{
		ROOM_STAT expected{ ROOM_STAT::AVAILABLE };
		mState.compare_exchange_strong(expected, ROOM_STAT::ROOM_IS_FULL); // this should not fail.
	}
}

void WaitRoom::DecreasePlayerCount()
{
	if (Empty() == false)
		mPlayerCount -= 1;

	if (Empty())
		CloseRoom();
}

bool WaitRoom::IsAdmin(int hostID) const
{
	return (gClients[hostID]->PlayerIndex == mAdminIndex);
}

void WaitRoom::SendGameStartFail(bool instSend)
{
	SC::packet_game_start_fail pck{};
	pck.size = sizeof(SC::packet_game_start_fail);
	pck.type = SC::GAME_START_FAIL;
	pck.room_id = mID;
	
	const int hostID = mPlayers[mAdminIndex]->ID;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) gClients[hostID]->SendMsg();
}

void WaitRoom::SendUpdatePlayerInfoToAll(int target, int ignore, bool instSend)
{
	const int idx = (int)gClients[target]->PlayerIndex;
	
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending player update packet to all.\n";
#endif
	SC::packet_update_player_info pck{};
	pck.size = sizeof(SC::packet_update_player_info);
	pck.type = SC::UPDATE_PLAYER_INFO;
	pck.room_id = mID;
	pck.player_idx = idx;
	pck.admin_idx = mAdminIndex;
	strncpy_s(pck.player_info.name, mPlayers[idx]->Name, MAX_NAME_SIZE - 1);
	pck.player_info.color = mPlayers[idx]->Color;
	pck.player_info.empty = mPlayers[idx]->Empty;
	pck.player_info.ready = mPlayers[idx]->Ready;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, ignore, instSend);	
}

void WaitRoom::SendRemovePlayerInfoToAll(int target, bool instSend)
{
	const int idx = (int)gClients[target]->PlayerIndex;
	
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending remove player packet to all.\n";
#endif
	SC::packet_remove_player pck{};
	pck.size = sizeof(SC::packet_remove_player);
	pck.type = SC::REMOVE_PLAYER;
	pck.player_idx = idx;
	pck.admin_idx = mAdminIndex;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, target, instSend);
}

void WaitRoom::SendUpdateMapInfoToAll(int ignore, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending map update packet to all.\n";
#endif
	SC::packet_update_map_info pck{};
	pck.size = sizeof(SC::packet_update_map_info);
	pck.type = SC::UPDATE_MAP_INFO;
	pck.room_id = mID;
	pck.map_id = mMapIndex;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, ignore, instSend);	
}

void WaitRoom::SendRoomInsideInfo(int id, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Sending room inside info packet.\n";
#endif
	SC::packet_room_inside_info pck{};
	pck.size = sizeof(SC::packet_room_inside_info);
	pck.type = SC::ROOM_INSIDE_INFO;
	pck.room_id = mID;
	pck.map_id = mMapIndex;
	pck.player_idx = gClients[id]->PlayerIndex;
	pck.admin_idx = mAdminIndex;
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		strncpy_s(pck.player_stats[i].name, mPlayers[i]->Name, MAX_NAME_SIZE - 1);
		pck.player_stats[i].color = mPlayers[i]->Color;
		pck.player_stats[i].empty = mPlayers[i]->Empty;
		pck.player_stats[i].ready = mPlayers[i]->Ready;
	}
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) gClients[id]->SendMsg();
}

void WaitRoom::SendRoomOutsideInfo(int id, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Sending room outside info packet.\n";
#endif
	SC::packet_room_outside_info pck{};
	pck.size = sizeof(SC::packet_room_outside_info);
	pck.type = SC::ROOM_OUTSIDE_INFO;
	pck.room_id = mID;
	pck.player_count = mPlayerCount;
	pck.game_started = GameRunning();
	pck.map_id = mMapIndex;
	pck.room_closed = Closed();
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[id]->SendMsg();
}

void WaitRoom::SendToAllPlayer(std::byte* pck, int size, int ignore, bool instSend)
{
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		if (mPlayers[i]->Empty == false && mPlayers[i]->ID != ignore)
		{
			int id = mPlayers[i]->ID;
			gClients[id]->PushPacket(pck, size);
			if (instSend) gClients[id]->SendMsg();
		}
	}
}


