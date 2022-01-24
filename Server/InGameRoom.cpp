#include "common.h"
#include "LoginServer.h"
#include "InGameRoom.h"
#include "Client.h"

InGameRoom::InGameRoom(int id, LoginServer* ptr)
	: mID(id), mGameRunning(false), mOpen(false),
	  mMapIndex(0), mPlayerCount(0), mLoginPtr(ptr)
{
	for (int i = 0; i < mPlayers.size(); i++)
		mPlayers[i] = PlayerInfo{ true, -1, false, -1, "" };
}

InGameRoom::~InGameRoom()
{
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
	auto p = std::find_if(mPlayers.begin(), mPlayers.end(),
		[](const PlayerInfo& info) { return (info.Empty == true); });
	if (p == mPlayers.end()) return false;

	Client* client = gClients[hostID].get();
	if (client->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
	{
		mLoginPtr->Disconnect(hostID);
		return false;
	}
	
	client->RoomID = mID;
	client->PlayerIndex = (int)std::distance(mPlayers.begin(), p);

	p->ID = hostID;
	p->Color = (char)std::distance(mPlayers.begin(), p);
	p->Empty = false;
	p->Name = client->Name;
	p->Ready = false;

	mPlayerCount.fetch_add(1);

	return true;
}

bool InGameRoom::RemovePlayer(int hostID)
{
	if (mOpen)
	{
		int idx = gClients[hostID]->PlayerIndex;
		if (idx < 0) return false;

		mPlayers[idx].Name = "";
		mPlayers[idx].Color = -1;
		mPlayers[idx].Empty = true;
		mPlayers[idx].ID = -1;
		mPlayers[idx].Ready = false;

		if (mPlayerCount > 0) mPlayerCount.fetch_sub(1);
		if (mPlayerCount == 0)
		{
			mOpen = false;
		}
		return true;
	}
	return false;
}

void InGameRoom::PrintWaitPlayers()
{
	int playerCount = mPlayerCount;
	for (int j = 0; j < playerCount;)
	{
		if (mPlayers[j].Empty == false)
		{
			std::cout << "\t" << j+1 << " .Name: " << mPlayers[j].Name << "\n";
			std::cout << "\t   Color: " << (int)mPlayers[j].Color << "\n";
			std::cout << "\t   Ready: " << std::boolalpha << mPlayers[j].Empty << "\n";
			std::cout << "\t   Host ID: " << mPlayers[j].ID << "\n\n";
			j += 1;
		}
	}
}

bool InGameRoom::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::SWITCH_MAP:
	{
		CS::packet_switch_map* pck = reinterpret_cast<CS::packet_switch_map*>(packet);
		if (pck->room_id == mID)
		{
			mMapIndex = pck->map_id;
			//gClients[id]->SendUpdateMapPacket();
		}
		else mLoginPtr->Disconnect(id);

		break;
	}
	case CS::PRESS_READY:
	{
		CS::packet_press_ready* pck = reinterpret_cast<CS::packet_press_ready*>(packet);
		if (pck->room_id == mID)
		{
			int idx = gClients[id]->PlayerIndex;
			if (idx < 0)
			{
				mLoginPtr->Disconnect(id);
				break;
			}
			mPlayers[idx].Ready = !mPlayers[idx].Ready;

			// Send all clients in this room..
		}
		else mLoginPtr->Disconnect(id);
		break;
	}
	default:
		std::cout << "Invalid packet.\n";
		return false;
	}
	return true;
}

void InGameRoom::SendUpdatePlayerInfoToAll(int target, int ignore, bool instSend)
{
	int idx = gClients[target]->PlayerIndex;
	if (idx < 0)
	{
		mLoginPtr->Disconnect(target);
		return;
	}

	std::cout << "[room id: " << mID << "] Sending player update packet to all.\n";
	SC::packet_update_player_info pck{};
	pck.size = sizeof(SC::packet_update_player_info);
	pck.type = SC::UPDATE_PLAYER_INFO;
	pck.room_id = mID;
	pck.index = idx;
	strncpy_s(pck.player_info.name, mPlayers[idx].Name.c_str(), MAX_NAME_SIZE - 1);
	pck.player_info.color = mPlayers[idx].Color;
	pck.player_info.empty = mPlayers[idx].Empty;
	pck.player_info.ready = mPlayers[idx].Ready;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, ignore, instSend);	
}

void InGameRoom::SendRemovePlayerInfoToAll(int target, bool instSend)
{
	int idx = gClients[target]->PlayerIndex;
	if (idx < 0)
	{
		mLoginPtr->Disconnect(target);
		return;
	}

	std::cout << "[room id: " << mID << "] Sending remove player packet to all.\n";
	SC::packet_remove_player pck{};
	pck.size = sizeof(SC::packet_remove_player);
	pck.type = SC::REMOVE_PLAYER;
	pck.index = idx;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, instSend);
}

void InGameRoom::SendUpdateMapInfoToAll(bool instSend)
{
	std::cout << "[room id: " << mID << "] Sending map update packet to all.\n";
	SC::packet_update_map_info pck{};
	pck.size = sizeof(SC::packet_update_map_info);
	pck.type = SC::UPDATE_MAP_INFO;
	pck.room_id = mID;
	pck.map_id = mMapIndex;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, instSend);	
}

void InGameRoom::SendRoomInsideInfo(int id, bool instSend)
{
	std::cout << "[" << id << "] Sending room inside info packet.\n";
	SC::packet_room_inside_info pck{};
	pck.size = sizeof(SC::packet_room_inside_info);
	pck.type = SC::ROOM_INSIDE_INFO;
	pck.room_id = mID;
	pck.map_id = mMapIndex;
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++) 
	{
		strncpy_s(pck.player_stats[i].name, mPlayers[i].Name.c_str(), MAX_NAME_SIZE - 1);
		pck.player_stats[i].color = mPlayers[i].Color;
		pck.player_stats[i].empty = mPlayers[i].Empty;
		pck.player_stats[i].ready = mPlayers[i].Ready;
	}
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) gClients[id]->SendMsg();
}

void InGameRoom::SendRoomOutsideInfo(int id, bool instSend)
{
	std::cout << "[" << id << "] Sending room outside info packet.\n";
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
