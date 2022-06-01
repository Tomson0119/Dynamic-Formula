#pragma once

#include "InGameServer.h"

class LoginServer;
class WaitRoom;
class Client;

class LobbyServer
{
public:
	using RoomList = std::array<std::unique_ptr<WaitRoom>, MAX_ROOM_SIZE>;
public:
	LobbyServer();
	~LobbyServer() = default;

	void Init(LoginServer* ptr);
	void TakeOverNewPlayer(int hostID);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	bool TryOpenRoom(int hostID);
	bool TryEnterRoom(int roomID, int hostID);
	void AcceptEnterRoom(int roomID, int hostID);
	void RevertScene(int hostID, bool logout=false);
	void PressStartOrReady(int roomID, int hostID);

	void SendRoomInfoToLobbyPlayers(int roomID, int ignore=-1, bool instSend = true);
	void SendExistingRoomList(int id, bool instSend=true);

	InGameServer& GetInGameServer() { return mInGameServer; }

private:
	int FindPageNumOfRoom(int roomId);
	int FindEmptyRoom();

public:
	void IncreasePlayerCount() { mLobbyPlayerCount.fetch_add(1); }
	void DecreasePlayerCount() { mLobbyPlayerCount.fetch_sub(1); }

private:
	std::atomic_int mRoomCount;
	std::atomic_int mLobbyPlayerCount;

	RoomList mRooms;
	/*std::deque<int> mOpenRoomIds;
	std::mutex mOpenRoomIdsMut;*/
	/*
		인게임서버는 여러 개가 존재할 수 있으며, 
		지정한 개수만큼의 방에 대해 로직을 계산하여 처리한다.
		실제로는 서버 컴퓨터가 하나이므로, 하나만 사용한다.
	*/
	InGameServer mInGameServer;
	LoginServer* mLoginPtr;
};