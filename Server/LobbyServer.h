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
		�ΰ��Ӽ����� ���� ���� ������ �� ������, 
		������ ������ŭ�� �濡 ���� ������ ����Ͽ� ó���Ѵ�.
		�����δ� ���� ��ǻ�Ͱ� �ϳ��̹Ƿ�, �ϳ��� ����Ѵ�.
	*/
	InGameServer mInGameServer;
	LoginServer* mLoginPtr;
};