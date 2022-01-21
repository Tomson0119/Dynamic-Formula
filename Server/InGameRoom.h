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
	char GetPlayerCount() const { return mPlayerCount; }
	bool Full() const { return (mPlayerCount == MAX_ROOM_CAPACITY); }
	bool Empty() const { return !mOpen; }

public:
	void SendAccessRoomAccept(int id, bool instSend=true);
	void SendCurrentRoomInfo(int id, bool instSend=true);
	void SendRoomInfoToLobbyPlayers(bool instSend=true);

private:
	int mID;
	std::atomic_int mPlayerCount;
	std::atomic_bool mOpen;
	std::atomic_bool mGameRunning;
	std::atomic_char mMapIndex;

	std::array<PlayerInfo, MAX_ROOM_CAPACITY> mPlayers;
	
	LobbyServer* mLobbyPtr;
};