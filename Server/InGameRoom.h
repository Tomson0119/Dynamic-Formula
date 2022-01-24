#pragma once

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	int ID;
	std::string Name;
};

class LoginServer;

class InGameRoom
{
public:
	InGameRoom(int id, LoginServer* ptr);
	~InGameRoom();

	bool OpenRoom(int hostID);	
	
	bool AddPlayer(int hostID);
	bool RemovePlayer(int hostID);

	// TEST
	void PrintWaitPlayers();

public:
	char GetPlayerCount() const { return mPlayerCount; }
	char GetMapIndex() const { return mMapIndex; }

	bool Full() const { return (Empty() == false && mPlayerCount == MAX_ROOM_CAPACITY); }
	bool Empty() const { return (mOpen == false); }
	bool GameRunning() const { return mGameRunning; }

public:
	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	void SendUpdatePlayerInfoToAll(int target, int ignore=-1, bool instSend=true);
	void SendRemovePlayerInfoToAll(int target, bool instSend=true);
	void SendUpdateMapInfoToAll(bool instSend=true);

	void SendRoomInsideInfo(int id, bool instSend=true);
	void SendRoomOutsideInfo(int id, bool instSend=true);

private:
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);


private:
	int mID;
	std::atomic_int mPlayerCount;
	std::atomic_bool mOpen;
	std::atomic_bool mGameRunning;
	std::atomic_char mMapIndex;

	std::array<PlayerInfo, MAX_ROOM_CAPACITY> mPlayers;

	LoginServer* mLoginPtr;
};