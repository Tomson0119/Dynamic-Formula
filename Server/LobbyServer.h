#pragma once

#include "DBHandler.h"

class Client;
class InGameRoom;

class LobbyServer
{
	friend class InGameRoom;
public:
	LobbyServer(const EndPoint& ep);
	virtual ~LobbyServer();

public:
	void Run();
	
	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes);
	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	static void NetworkThreadFunc(LobbyServer& server);
	static const int MaxThreads = 1;

private:
	int GetAvailableID();
	void ProcessLoginStep(const char* name, int id);
	void SendCurrentRoomList(int id);

private:
	Socket mListenSck;
	IOCP mIOCP;

	std::vector<std::thread> mThreads;
	std::atomic_bool mLoop;	
	std::atomic_int mRoomCount;
	std::atomic_int mLobbyPlayerCount;

	DBHandler mDBHandler;

	static std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;
	static std::array<std::unique_ptr<InGameRoom>, MAX_ROOM_SIZE> gRooms;
};