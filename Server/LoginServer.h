#pragma once

#include "LobbyServer.h"
#include "DBHandler.h"

class Client;

extern std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;

class LoginServer
{
public:
	LoginServer(const EndPoint& ep);
	~LoginServer() = default;

public:
	void Run();
	
	void AcceptLogin(const char* name, int id);
	void Logout(int id);

	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);
	int GetAvailableID();

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes);
	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	static void NetworkThreadFunc(LoginServer& server);
	static const int MAX_THREADS = 4;

private:
	Socket mListenSck;
	IOCP mIOCP;

	std::map<std::thread::id, int> mThreadIDs;
	std::array<DBHandler, MAX_THREADS> mDBHandlers;
	std::vector<std::thread> mThreads;
	std::atomic_bool mLoop;		

	/*
		로비서버는 분산 서버 시스템에서 여러 개가 존재할 수 있다.
		따라서 스케일 아웃이 가능한 디자인으로 생성해야 한다.
		하지만 실제로는 로드 밸런서가 없기 때문에 1개만 사용한다.
	*/
	LobbyServer mLobby;
};