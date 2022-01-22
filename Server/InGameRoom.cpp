#include "common.h"
#include "InGameRoom.h"
#include "LobbyServer.h"
#include "Client.h"

InGameRoom::InGameRoom(int id, LobbyServer* server)
	: mID(id), mGameRunning(false), mOpen(false),
	  mMapIndex(0), mLobbyPtr(server), mPlayerCount(0)
{
	for (int i = 0; i < mPlayers.size(); i++)
		mPlayers[i] = PlayerInfo{ true, -1, false, -1, "" };
}

InGameRoom::~InGameRoom()
{
}

void InGameRoom::OpenRoom(int hostID)
{
	bool b = false;
	if (mOpen.compare_exchange_strong(b, true) == false)
		std::cout << "Room has already created.\n";
	else 
		AddPlayer(hostID);
}

void InGameRoom::TryAddPlayer(int hostID)
{
	Client* client = mLobbyPtr->gClients[hostID].get();
	if (Full())
		client->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_FULL);
	else if (Empty())
		client->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_CLOSED);
	else if (GameRunning())
		client->SendAccessRoomDeny(ROOM_STAT::GAME_STARTED);
	else
		AddPlayer(hostID);
}

void InGameRoom::AddPlayer(int hostID)
{
	if (mOpen && Full() == false)
	{
		mPlayerCount.fetch_add(1);

		auto p = std::find_if(mPlayers.begin(), mPlayers.end(),
			[](const PlayerInfo& info) { return (info.Empty); });
		
		Client* client = mLobbyPtr->gClients[hostID].get();
		client->AssignedRoomID = mID;

		p->ID = hostID;
		p->Color = 0;
		p->Empty = false;
		p->Name = client->Name;
		p->Ready = false;

		if (client->ChangeState(CLIENT_STAT::LOBBY, CLIENT_STAT::IN_ROOM) == false)
			mLobbyPtr->Disconnect(hostID);
		
		mLobbyPtr->mLobbyPlayerCount.fetch_sub(1);
		SendPlayersInfo(hostID);
		// SendNewPlayerInfo();
		SendRoomInfoToLobbyPlayers();
	}
}

void InGameRoom::RemovePlayer(int hostID)
{
	if (mOpen)
	{
		mPlayerCount.fetch_sub(1);
		if (mPlayerCount == 0)
		{
			mOpen = false;
			mLobbyPtr->mRoomCount.fetch_sub(1);
		}

		Client* client = mLobbyPtr->gClients[hostID].get();
		client->AssignedRoomID = -1;
		
		auto p = std::find_if(mPlayers.begin(), mPlayers.end(),
			[&client](const PlayerInfo& info) { return (info.ID == client->ID); });
		
		p->Empty = true;
		
		mLobbyPtr->mLobbyPlayerCount.fetch_add(1);
		// SendRemovedPlayerID();
		SendRoomInfoToLobbyPlayers(hostID);

		if (client->ChangeState(CLIENT_STAT::IN_ROOM, CLIENT_STAT::LOBBY) == false)
			mLobbyPtr->Disconnect(hostID);
	}
}

void InGameRoom::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	default:
		std::cout << "Invalid packet.\n";
		break;
	}
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
	mLobbyPtr->gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) mLobbyPtr->gClients[id]->SendMsg();
}

void InGameRoom::SendRoomInfoToLobbyPlayers(bool instSend)
{
	int lobbyPlayers = mLobbyPtr->mLobbyPlayerCount;
	for (int i = 0; i < MAX_PLAYER_SIZE && lobbyPlayers > 0; i++)
	{
		Client* client = mLobbyPtr->gClients[i].get();
		if (client->GetCurrentState() == CLIENT_STAT::LOBBY)
		{
			SendCurrentRoomInfo(i);
			lobbyPlayers -= 1;
		}
	}
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
	mLobbyPtr->gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) mLobbyPtr->gClients[id]->SendMsg();
}
