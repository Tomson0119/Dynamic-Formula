#pragma once

struct PlayerInfo
{
	bool Empty;
	char CarModel;
	bool Ready;
	int ID;
	std::string Name;
};

class LobbyServer;

class InGameRoom
{
public:
	InGameRoom(int id, LobbyServer* server);
	~InGameRoom();

public:
	char GetPlayerCount() const { return mPlayerCount; }
	bool Full() const { return (mPlayerCount == MAX_ROOM_CAPACITY); }
	bool Empty() const { return mOpen; }

private:
	int mID;
	bool mOpen;
	int mPlayerCount;

	std::array<PlayerInfo, MAX_ROOM_CAPACITY> mPlayers;
	
	LobbyServer* mLobbyPtr;
};