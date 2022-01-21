#include "common.h"
#include "InGameRoom.h"
#include "LobbyServer.h"
#include "Client.h"

InGameRoom::InGameRoom(int id, LobbyServer* server)
	: mID(id), mOpen(false), mGameRunning(false),
	  mMapIndex(0), mLobbyPtr(server), mPlayerCount(0)
{
	for (int i = 0; i < mPlayers.size(); i++)
		mPlayers[i] = PlayerInfo{ true, -1, false, -1, "" };
}

InGameRoom::~InGameRoom()
{
}

void InGameRoom::OpenRoom(int player)
{
	bool b = false;
	if (mOpen.compare_exchange_strong(b, true) == false)
		std::cout << "Room has already created.\n";
	else 
		AddPlayer(player);
}

void InGameRoom::AddPlayer(int player)
{
	if (mOpen)
	{
		mPlayerCount.fetch_add(1);
		
		Client* client = mLobbyPtr->gClients[player].get();
		PlayerInfo& info = mPlayers[mPlayerCount - 1];
		info.ID = player;
		info.Color = 0;
		info.Empty = false;
		info.Name = client->Name;
		info.Ready = false;

		if (client->ChangeState(CLIENT_STAT::LOGIN, CLIENT_STAT::IN_ROOM) == false)
			client->Disconnect();

		SendAccessRoomAccept(player);
		SendRoomInfoToLobbyPlayers();
	}
}

void InGameRoom::SendAccessRoomAccept(int id, bool instSend)
{
	std::cout << "[" << id << "] sending access room accept packet.\n";
	SC::packet_access_room_accept pck{};
	pck.size = sizeof(SC::packet_access_room_accept);
	pck.type = SC::ACCESS_ROOM_ACCEPT;
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
	for (int i = 0; i < MAX_PLAYER_SIZE; i++)
	{
		Client* client = mLobbyPtr->gClients[i].get();
		if (client->GetCurrentState() == CLIENT_STAT::LOGIN)
		{
			// TODO:
		}
	}
}

void InGameRoom::SendCurrentRoomInfo(int id, bool instSend)
{
	SC::packet_room_info pck{};
	pck.size = sizeof(SC::packet_room_info);
	pck.type = SC::ROOM_INFO;
	pck.room_id = mID;
	pck.player_count = mPlayerCount;
	pck.game_started = mGameRunning;
	pck.map_id = mMapIndex;
	mLobbyPtr->gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) mLobbyPtr->gClients[id]->SendMsg();
}
