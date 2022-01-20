#pragma once

struct PlayerInfo
{
	bool Empty;
	char Color;
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

	void OpenRoom(int player);
	void AddPlayer(int player);

public:
	char GetPlayerCount() const { return mPlayerCount.load(); }
	bool Full() const { return (mPlayerCount.load() == MAX_ROOM_CAPACITY); }
	bool Empty() const { return mOpen.load(); }

private:
	void SendAccessRoomAcceptPacket(int id);
	void SendRoomInfoToLobbyPlayers();

private:
	int mID;
	std::atomic_int mPlayerCount;
	std::atomic_bool mOpen;

	std::array<PlayerInfo, MAX_ROOM_CAPACITY> mPlayers;
	
	LobbyServer* mLobbyPtr;
};