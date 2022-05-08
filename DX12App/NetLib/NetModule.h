#pragma once

#include "NetClient.h"

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	char Name[MAX_NAME_SIZE];
	XMFLOAT3 StartPosition;
	XMFLOAT4 StartRotation;
};

class Scene;

class NetModule
{
	using PlayerList = std::array<PlayerInfo, MAX_ROOM_CAPACITY>;
public:
	NetModule();
	~NetModule();

	bool Connect(const char* ip, short port);
	void PostDisconnect();

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes, int id);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int bytes);

	static void NetworkFunc(NetModule& net);

public:
	void InitRoomInfo(SC::packet_room_inside_info* pck);
	void RemovePlayer(SC::packet_remove_player* pck);
	void UpdateMapIndex(SC::packet_update_map_info* pck);
	void UpdatePlayerInfo(SC::packet_update_player_info* pck);

	void InitPlayerTransform(SC::packet_game_start_success* pck);

public:
	void SetInterface(Scene* scenePtr) { mScenePtr = scenePtr; }
	NetClient* Client() const { return mNetClient.get(); }

	void SetRoomID(int roomID) { mRoomID = roomID; }	
	int GetRoomID() const { return mRoomID; }

	char GetPlayerIndex() const { return mPlayerIdx; }
	char GetMapIndex() const { return mMapIdx; }
	char GetAdminIndex() const { return mAdminIdx; }

	bool IsAdmin() const { return mPlayerIdx == mAdminIdx; }
	const PlayerList& GetPlayersInfo() const { return mPlayerList; }

	std::mutex& GetPlayerListMutex() { return mPlayerListMut; }

	void SetLatency(uint64_t sendTime);
	void SetUpdateRate();

	float GetLatency() const { return (float)mLatency / 1000.0f; }
	float GetUpdateRate() const { return (float)mUpdateRate / 1000.0f; }

	void SetServerIP(const std::string& ip) { mServerIPAddress = ip; }
	const std::string& GetServerIP() const { return mServerIPAddress; }
	
private:
	void Init();

private:
	std::atomic_bool mLoop;

	std::atomic_int mRoomID;
	std::atomic_char mPlayerIdx;
	std::atomic_char mAdminIdx;
	std::atomic_char mMapIdx;

	PlayerList mPlayerList;
	std::mutex mPlayerListMut;

	std::unique_ptr<NetClient> mNetClient;
	std::thread mNetThread;

	Clock::time_point mTimeStamp;
	std::atomic_uint64_t mLatency;
	std::atomic_uint64_t mUpdateRate;

	IOCP mIOCP;
	Scene* mScenePtr;

	std::string mServerIPAddress;
};
