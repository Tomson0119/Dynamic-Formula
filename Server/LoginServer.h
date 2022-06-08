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
		�κ񼭹��� �л� ���� �ý��ۿ��� ���� ���� ������ �� �ִ�.
		���� ������ �ƿ��� ������ ���������� �����ؾ� �Ѵ�.
		������ �����δ� ���� ��ǻ�Ͱ� �ϳ��̹Ƿ�, �ϳ��� ����Ѵ�.
	*/
	LobbyServer mLobby;
};