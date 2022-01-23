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

bool InGameRoom::TryAddPlayer(int hostID)
{
	Client* client = gClients[hostID].get();
	if (Full())
	{
		client->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_FULL);
		return false;
	}
	else if (Empty())
	{
		client->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_CLOSED);
		return false;
	}
	else if (GameRunning())
	{
		client->SendAccessRoomDeny(ROOM_STAT::GAME_STARTED);
		return false;
	}
	else return AddPlayer(hostID);
}

bool InGameRoom::AddPlayer(int hostID)
{
	auto p = std::find_if(mPlayers.begin(), mPlayers.end(),
		[](const PlayerInfo& info) { return (info.Empty); });		

	if (p == mPlayers.end()) return false;

	Client* client = gClients[hostID].get();
	if (client->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
	{
		mLoginPtr->Disconnect(hostID);
		return false;
	}
	
	client->RoomID = mID;

	p->ID = hostID;
	p->Color = 0;
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
		auto p = std::find_if(mPlayers.begin(), mPlayers.end(),
			[&hostID](const PlayerInfo& info) { return (info.ID == hostID); });

		if (p == mPlayers.end()) return false;

		p->Empty = true;

		if (mPlayerCount > 0) mPlayerCount.fetch_sub(1);
		if (mPlayerCount == 0)
		{
			mOpen = false;
		}
		else
		{
			// Send player update packet to clients in this room..
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
	default:
		std::cout << "Invalid packet.\n";
		return false;
	}
	return true;
}

void InGameRoom::SendPlayersInfo(int id, bool instSend)
{
	std::cout << "[" << id << "] Sending wait players info packet.\n";
	SC::packet_wait_players_info pck{};
	pck.size = sizeof(SC::packet_wait_players_info);
	pck.type = SC::WAIT_PLAYERS_INFO;
	pck.room_id = mID;
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++) {
		strncpy_s(pck.player_stats[i].name, mPlayers[i].Name.c_str(), mPlayers[i].Name.size());
		pck.player_stats[i].color = mPlayers[i].Color;
		pck.player_stats[i].empty = mPlayers[i].Empty;
		pck.player_stats[i].ready = mPlayers[i].Ready;
	}
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) gClients[id]->SendMsg();
}

void InGameRoom::SendCurrentRoomInfo(int id, bool instSend)
{
	std::cout << "[" << id << "] Sending current room info packet.\n";
	SC::packet_room_update_info pck{};
	pck.size = sizeof(SC::packet_room_update_info);
	pck.type = SC::ROOM_UPDATE_INFO;
	pck.room_id = mID;
	pck.player_count = mPlayerCount;
	pck.game_started = mGameRunning;
	pck.map_id = mMapIndex;
	pck.room_closed = !mOpen;
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[id]->SendMsg();
}
