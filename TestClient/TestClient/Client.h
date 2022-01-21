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

	void PushScene(SCENE newScene) { mSceneStack.push(newScene); }
	void PopScene() { mSceneStack.pop(); }

	SCENE GetCurrentScene() const { return mSceneStack.top(); }
	SOCKET GetSocket() const { return m_socket.GetSocket(); }

	bool SceneEmpty() const { return mSceneStack.empty(); }
	bool Admin() const { return m_isAdmin; }

public:
	void EnterLoginScreen();
	void EnterLobbyScreen();
	void EnterWaitRoomScreen();
	void EnterInGameScreen();
	
	void RequestLogin(const std::string& name, const std::string& pwd);
	void RequestRegister(const std::string& name, const std::string& pwd);

	void RequestNewRoom();
	void RequestEnterRoom(int room_id);

	void BackToLobby();
	void SwitchMap();
	void SetOrUnsetReady();

public:
	int ID;
	int RoomID;
	
	std::atomic_bool LoginSuccessFlag;
	std::atomic_bool RecvResultFlag;
	std::atomic_bool RoomEnteredFlag;
	std::atomic_bool GameStartFlag;
;
private:
	Socket m_socket;

	std::stack<SCENE> mSceneStack;
	std::atomic_bool m_isAdmin;

	WSAOVERLAPPEDEX* m_sendOverlapped;
	WSAOVERLAPPEDEX m_recvOverlapped;
};
