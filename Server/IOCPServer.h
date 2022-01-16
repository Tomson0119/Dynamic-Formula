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

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes);
	bool ProcessPacket(std::byte* packet, int id, int bytes);

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