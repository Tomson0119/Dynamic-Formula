#include "common.h"
#include "InGameRoom.h"
#include "IOCPServer.h"

InGameRoom::InGameRoom(int id, LobbyServer* server)
	: mID(id), mOpen(false), mLobbyPtr(server), mPlayerCount(0)
{
	for (int i = 0; i < mPlayers.size(); i++)
		mPlayers[i] = PlayerInfo{ -1, true, 0, 0, 0 };
}

InGameRoom::~InGameRoom()
{
}
