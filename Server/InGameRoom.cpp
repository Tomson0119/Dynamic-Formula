#include "common.h"
#include "InGameRoom.h"
#include "IOCPServer.h"
#include "Session.h"

InGameRoom::InGameRoom(int id, LobbyServer* server)
	: mID(id), mOpen(false), mLobbyPtr(server), mPlayerCount(0)
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
	if (mOpen.compare_exchange_strong(b, true) == false) {
		std::cout << "Room has already created.\n";
	}
	else {
		AddPlayer(player);
	}		
}

void InGameRoom::AddPlayer(int player)
{
	if (mOpen.load())
	{
		mPlayerCount.fetch_add(1);
		
		PlayerInfo& info = mPlayers[mPlayerCount.load() - 1];
		info.ID = player;
		info.CarModel = 0;
		info.Empty = false;
		info.Name = mLobbyPtr->gClients[player]->Name;
		info.Ready = false;

		SendAccessRoomAcceptPacket(player);
	}
}

void InGameRoom::SendAccessRoomAcceptPacket(int id)
{
	std::cout << "[" << id << "] sending access room accept packet.\n";
	SC::packet_access_room_accept pck{};
	pck.size = sizeof(SC::packet_access_room_accept);
	pck.type = SC::ACCESS_ROOM_ACCEPT;
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++) {
		strncpy_s(pck.player_stats[i].name, mPlayers[i].Name.c_str(), mPlayers[i].Name.size());
		pck.player_stats[i].color = mPlayers[i].CarModel;
		pck.player_stats[i].empty = mPlayers[i].Empty;
		pck.player_stats[i].ready = mPlayers[i].Ready;
	}
	mLobbyPtr->gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	mLobbyPtr->gClients[id]->SendMsg();
}
