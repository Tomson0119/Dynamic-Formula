#pragma once

#include "LobbyServer.h"
#include "DBHandler.h"
#include "UDPReceiver.h"

class Client;

extern std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;

class LoginServer
{
public:
	LoginServer(const EndPoint& ep);
	~LoginServer();

public:
	void Run();
	
	void AcceptLogin(const char* name, int id);
	void Logout(int id);

	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck, sockaddr_in* remote);
	int GetAvailableID();

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes);
	bool ProcessPacket(std::byte* packet, const CS::PCK_TYPE& type, int id, int bytes);

	static void SignalHandler(int signal);
	static void NetworkThreadFunc(LoginServer& server);	

	IOCP& GetIOCP() { return msIOCP; }

private:
	Socket mListenSck;
	UDPReceiver mUDPReceiver;

	std::map<std::thread::id, int> mThreadIdMap;

	static IOCP msIOCP;

	std::array<DBHandler, MAX_THREAD_COUNT> mDBHandlers;
	std::array<std::thread, MAX_THREAD_COUNT> mThreads;

	/*
		로비서버는 분산 서버 시스템에서 여러 개가 존재할 수 있다.
		따라서 스케일 아웃이 가능한 디자인으로 생성해야 한다.
		하지만 실제로는 서버 컴퓨터가 하나이므로, 하나만 사용한다.
	*/
	LobbyServer mLobby;
};