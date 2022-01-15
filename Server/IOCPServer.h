#pragma once

#include "Session.h"
#include "DBHandler.h"

class IOCPServer
{
public:
	IOCPServer(const EndPoint& ep);
	virtual ~IOCPServer();

public:
	void Run();
	
	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);

	void ProcessPackets(int id, std::byte* packet);
	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);

	void AssemblePacket(int client_id, WSAOVERLAPPEDEX* over, int bytes);

	static void NetworkThreadFunc(IOCPServer& server);
	static const int MaxThreads = 1;

private:
	int GetAvailableID();

private:
	Socket mListenSck;
	IOCP mIOCP;

	std::vector<std::thread> mThreads;
	std::atomic_bool mLoop;

	DBHandler mDBHandler;

	static std::array<std::unique_ptr<Session>, MAX_PLAYER> gClients;
};