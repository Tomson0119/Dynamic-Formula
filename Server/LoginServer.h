#pragma once

#include "LobbyServer.h"
#include "DBHandler.h"

class Client;
class InGameRoom;

extern std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;

class LoginServer
{
public:
	LoginServer(const EndPoint& ep);
	virtual ~LoginServer();

public:
	void Run();
	
	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);
	int GetAvailableID();

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes);
	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	static void NetworkThreadFunc(LoginServer& server);
	static const int MaxThreads = 4;

private:
	Socket mListenSck;
	IOCP mIOCP;

	std::vector<std::thread> mThreads;
	std::atomic_bool mLoop;		

	LobbyServer mLobby;
	DBHandler mDBHandler;
};