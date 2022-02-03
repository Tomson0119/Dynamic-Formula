#pragma once

#include "NetClient.h"

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	char Name[MAX_NAME_SIZE];
};

class Scene;

class NetModule
{
	using PlayerList = std::array<PlayerInfo, MAX_ROOM_CAPACITY>;
public:
	NetModule();
	~NetModule();

	bool Connect(const char* ip, short port);
	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int bytes);

	static void NetworkFunc(NetModule& net);

public:
	void InitRoomInfo(SC::packet_room_inside_info* pck);
	void RemovePlayer(SC::packet_remove_player* pck);
	void UpdateMapIndex(SC::packet_update_map_info* pck);
	void UpdatePlayerInfo(SC::packet_update_player_info* pck);

	void SetInterface(Scene* scenePtr) { mScenePtr = scenePtr; }
	NetClient* Client() const { return mNetClient.get(); }

	void SetRoomID(int roomID) { mRoomID = roomID; }
	int GetRoomID() const { return mRoomID; }

	bool IsAdmin() const { return mPlayerIdx == mAdminIdx; }

private:
	void Init();

private:
	std::atomic_bool mLoop;

	std::atomic_int mRoomID;
	std::atomic_int mPlayerIdx;
	std::atomic_int mAdminIdx;
	std::atomic_char mMapIdx;

	PlayerList mPlayerList;

	std::unique_ptr<NetClient> mNetClient;
	std::thread mNetThread;

	IOCP mIOCP;
	Scene* mScenePtr;
};
