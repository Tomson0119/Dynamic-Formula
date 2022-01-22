#pragma once

#include <stack>
#include "common.h"

enum class SCENE : char
{
	LOGIN=0,
	LOBBY,
	ROOM,
	IN_GAME
};

struct Room
{
	int ID;
	char PlayerCount;
	char MapID;
	bool GameStarted;
	bool Closed;
};

class Client
{
public:
	Client(int id);
	~Client();

	bool Connect(const std::string& addr, short port);
	void Disconnect();

	void PushPacket(std::byte* pck, int bytes);

	void Send();
	void Send(std::byte* msg, int bytes);

	void Recv();

	void PushScene(SCENE newScene);
	void PopScene();

	SCENE GetCurrentScene();
	SOCKET GetSocket() const { return m_socket.GetSocket(); }

	bool SceneEmpty() const { return mSceneStack.empty(); }
	bool Admin() const { return m_isAdmin; }

public:
	void InsertRoom(SC::packet_room_update_info* packet);
	void EraseRoom(int room_id);
	void PrintRoomList();

public:
	void ShowLoginScreen();

	void PrintLobbyInterface();
	void ShowLobbyScreen();
	void ShowWaitRoomScreen();
	void ShowInGameScreen();
	
	void RequestLogin(const std::string& name, const std::string& pwd);
	void RequestRegister(const std::string& name, const std::string& pwd);

	void RequestNewRoom();
	void RequestEnterRoom(int room_id);

	void RevertScene();
	void SwitchMap();
	void SetOrUnsetReady();

public:
	int ID;
	std::atomic_int RoomID;

private:
	Socket m_socket;

	std::mutex mSceneStackLock;
	std::stack<SCENE> mSceneStack;
	std::atomic_bool m_isAdmin;

	std::mutex mRoomListLock;
	std::unordered_map<int, Room> mRoomList;

	WSAOVERLAPPEDEX* m_sendOverlapped;
	WSAOVERLAPPEDEX m_recvOverlapped;
};