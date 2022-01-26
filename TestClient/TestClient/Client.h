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

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	std::string Name;
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

public:
	void InsertRoom(SC::packet_room_outside_info* packet);
	void EraseRoom(int room_id);
	void PrintRoomList();
	void ClearRoomList();

	void UpdateWaitRoomInfo(SC::packet_room_inside_info* info);
	void UpdatePlayer(int idx, SC::PlayerInfo& state);
	void RemovePlayer(int idx);
	void UpdateMap(int map_id);

	void PrintWaitRoomInfo();
	void ClearPlayerList();

public:
	void ShowLoginScreen();

	void PrintLobbyInterface();
	void ShowLobbyScreen();
	void PrintWaitRoomInterface();
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
	std::atomic_int AdminIdx;
	std::atomic_int PlayerIdx;

	std::string LoginResult;
	std::string EnterRoomResult;
	std::string GameStartResult;

private:
	Socket m_socket;

	std::mutex mSceneStackLock;
	std::stack<SCENE> mSceneStack;
	std::atomic_char mMapIdx;

	std::atomic_bool mEnteredRoomFlag;
	std::atomic_bool mEnteredLobbyFlag;

	std::mutex mRoomListLock;
	std::unordered_map<int, Room> mRoomList;

	std::array<PlayerInfo, MAX_ROOM_CAPACITY> mPlayerList;

	WSAOVERLAPPEDEX* m_sendOverlapped;
	WSAOVERLAPPEDEX m_recvOverlapped;
};