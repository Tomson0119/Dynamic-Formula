#pragma once

#include "BPHandler.h"

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	int ID;
	char Name[MAX_NAME_SIZE];
};

class LoginServer;

class InGameRoom
{
	using PlayerList = std::array<PlayerInfo, MAX_ROOM_CAPACITY>;
public:
	InGameRoom(int id, LoginServer* ptr);
	~InGameRoom() = default;

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
	void SendUpdateMapInfoToAll(int ignore=-1, bool instSend=true);

	void SendRoomInsideInfo(int id, bool instSend=true);
	void SendRoomOutsideInfo(int id, bool instSend=true);

private:
	void GameStartIfAllReady(int admin, bool instSend=true);
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);

private:
	int mID;
	std::atomic_char mPlayerCount;
	std::atomic_bool mOpen;
	std::atomic_bool mGameRunning;
	std::atomic_char mMapIndex;

	std::atomic_char mAdminIndex;

	PlayerList mPlayers;
	LoginServer* mLoginPtr;
	BPHandler mPhysicsEngine;
};