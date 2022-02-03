#include "common.h"
#include "LoginServer.h"
#include "InGameRoom.h"
#include "Client.h"

InGameRoom::InGameRoom(int id, LoginServer* ptr)
	: mID(id), mGameRunning(false), mOpen(false),
	  mMapIndex(0), mPlayerCount(0), mAdminIndex(-1),
	  mLoginPtr(ptr)
{
	for (int i = 0; i < mPlayers.size(); i++)
		mPlayers[i] = PlayerInfo{ true, -1, false, -1, "" };
	
	mPhysicsEngine.Init(-10.0f);
}

bool InGameRoom::OpenRoom(int hostID)
{
	if (Empty() && AddPlayer(hostID))
	{
		bool b = false;
		mOpen.compare_exchange_strong(b, true);

		return true;
	}
	return false;
}

bool InGameRoom::AddPlayer(int hostID)
{
	const int idx = [&] {
		for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
			if (mPlayers[i].Empty)
				return i;
		return -1;
	}();

	if (idx < 0) return false;

	Client* client = gClients[hostID].get();
	if (client->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
	{
		mLoginPtr->Disconnect(hostID);
		return false;
	}
	
	client->RoomID = mID;
	client->PlayerIndex = (char)idx;
	if (mPlayerCount == 0) mAdminIndex = idx;

	mPlayers[idx].ID = hostID;
	mPlayers[idx].Color = (char)idx;
	mPlayers[idx].Empty = false;
	mPlayers[idx].Ready = false;
	strncpy_s(mPlayers[idx].Name, client->Name.c_str(), MAX_NAME_SIZE - 1);

	mPlayerCount.fetch_add(1);

	return true;
}

bool InGameRoom::RemovePlayer(int hostID)
{
	if (mOpen)
	{
		int idx = (int)gClients[hostID]->PlayerIndex;
		if (idx < 0) return false;

		mPlayers[idx].Name[0] = 0;
		mPlayers[idx].Color = -1;
		mPlayers[idx].Empty = true;
		mPlayers[idx].ID = -1;
		mPlayers[idx].Ready = false;

		if (mPlayerCount > 0) mPlayerCount.fetch_sub(1);

		if (mPlayerCount == 0)
			mOpen = false;
		else if (idx == mAdminIndex)
		{
			mAdminIndex = [&] {
				for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
					if (mPlayers[i].Empty == false) 
						return i;
				return -1;
			}();
			if (mAdminIndex < 0) return false;
		}
		return true;
	}
	return false;
}

void InGameRoom::PrintWaitPlayers()
{
	int playerCount = mPlayerCount;
	for (int j = 0; j < MAX_ROOM_CAPACITY && playerCount > 0; j++)
	{
		if (mPlayers[j].Empty == false)
		{
			std::cout << "\t" << j+1 << " .Name: " << mPlayers[j].Name << "\n";
			std::cout << "\t   Color: " << (int)mPlayers[j].Color << "\n";
			std::cout << "\t   Ready: " << std::boolalpha << mPlayers[j].Empty << "\n";
			std::cout << "\t   Host ID: " << mPlayers[j].ID << "\n\n";
			playerCount -= 1;
		}
	}
}

bool InGameRoom::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::SWITCH_MAP:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received switch map packet.\n";
#endif
		CS::packet_switch_map* pck = reinterpret_cast<CS::packet_switch_map*>(packet);
		if (pck->room_id != mID || gClients[id]->PlayerIndex < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}		
		if (gClients[id]->PlayerIndex == mAdminIndex)
		{
			mMapIndex = pck->map_id;
			SendUpdateMapInfoToAll(id);
		}		
		break;
	}
	case CS::PRESS_READY:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received press ready packet.\n";
#endif
		CS::packet_press_ready* pck = reinterpret_cast<CS::packet_press_ready*>(packet);
		
		// for stress test
		gClients[id]->ReadySendTime = pck->send_time;

		if (pck->room_id != mID || gClients[id]->PlayerIndex < 0)
		{
			mLoginPtr->Disconnect(id);
			break;
		}
		int idx = (int)gClients[id]->PlayerIndex;
		if (idx == mAdminIndex)
		{
			GameStartIfAllReady(idx);
		}
		else
		{
			mPlayers[idx].Ready = !mPlayers[idx].Ready;
			SendUpdatePlayerInfoToAll(id, id);
		}
		break;
	}
	default:
		std::cout << "Invalid packet.\n";
		return false;
	}
	return true;
}

void InGameRoom::GameStartIfAllReady(int admin, bool instSend)
{
	bool allReady = [&] {
		for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
		{
			if (i == admin) continue;
			if (mPlayers[i].Empty == false && mPlayers[i].Ready == false)
				return false;
		}
		return (mPlayerCount > 1);
	}();

	SC::packet_game_start_result pck{};
	pck.size = sizeof(SC::packet_game_start_result);
	pck.type = SC::GAME_START_RESULT;
	pck.room_id = mID;
	pck.succeeded = allReady;
	// for stress test
	pck.send_time = gClients[mPlayers[admin].ID]->ReadySendTime;
	
	if (allReady)
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[room id: " << mID << "] Sending game start packet to all.\n";
#endif
		SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, -1, instSend);
	}
	else
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << admin << "] Sending game start fail packet.\n";
#endif
		gClients[admin]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
		gClients[admin]->SendMsg();
	}
}

void InGameRoom::SendUpdatePlayerInfoToAll(int target, int ignore, bool instSend)
{
	int idx = (int)gClients[target]->PlayerIndex;
	if (idx < 0)
	{
		mLoginPtr->Disconnect(target);
		return;
	}

#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending player update packet to all.\n";
#endif
	SC::packet_update_player_info pck{};
	pck.size = sizeof(SC::packet_update_player_info);
	pck.type = SC::UPDATE_PLAYER_INFO;
	pck.room_id = mID;
	pck.player_idx = idx;
	pck.admin_idx = mAdminIndex;
	strncpy_s(pck.player_info.name, mPlayers[idx].Name, MAX_NAME_SIZE - 1);
	pck.player_info.color = mPlayers[idx].Color;
	pck.player_info.empty = mPlayers[idx].Empty;
	pck.player_info.ready = mPlayers[idx].Ready;
	// for stress test
	pck.send_time = gClients[mPlayers[idx].ID]->ReadySendTime;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, ignore, instSend);	
}

void InGameRoom::SendRemovePlayerInfoToAll(int target, bool instSend)
{
	int idx = (int)gClients[target]->PlayerIndex;
	if (idx < 0)
	{
		mLoginPtr->Disconnect(target);
		return;
	}

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

void InGameRoom::SendUpdateMapInfoToAll(int ignore, bool instSend)
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

void InGameRoom::SendRoomInsideInfo(int id, bool instSend)
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
		strncpy_s(pck.player_stats[i].name, mPlayers[i].Name, MAX_NAME_SIZE - 1);
		pck.player_stats[i].color = mPlayers[i].Color;
		pck.player_stats[i].empty = mPlayers[i].Empty;
		pck.player_stats[i].ready = mPlayers[i].Ready;
	}
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) gClients[id]->SendMsg();
}

void InGameRoom::SendRoomOutsideInfo(int id, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Sending room outside info packet.\n";
#endif
	SC::packet_room_outside_info pck{};
	pck.size = sizeof(SC::packet_room_outside_info);
	pck.type = SC::ROOM_OUTSIDE_INFO;
	pck.room_id = mID;
	pck.player_count = mPlayerCount;
	pck.game_started = mGameRunning;
	pck.map_id = mMapIndex;
	pck.room_closed = !mOpen;
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[id]->SendMsg();
}

void InGameRoom::SendToAllPlayer(std::byte* pck, int size, int ignore, bool instSend)
{
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		if (mPlayers[i].Empty == false && mPlayers[i].ID != ignore)
		{
			int id = mPlayers[i].ID;
			gClients[id]->PushPacket(pck, size);
			if (instSend) gClients[id]->SendMsg();
		}
	}
}