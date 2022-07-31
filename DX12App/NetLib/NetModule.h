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

struct Room
{
	int ID;
	unsigned char PlayerCount;
	unsigned char MapID;
	bool GameStarted;
	bool Opened;
};

class Scene;

class NetModule
{
	using PlayerList = std::array<PlayerInfo, MAX_ROOM_CAPACITY>;
	using RoomList = std::array<Room, ROOM_NUM_PER_PAGE>;
public:
	NetModule();
	~NetModule();

	void StarttHolePunching();

	bool Connect(const std::string& ip, u_short port);
	void PostDisconnect();

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes, int id);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int bytes);
	bool ProcessPacket(std::byte* packet, const SC::PCK_TYPE& type, int bytes);

	static void NetworkFunc(NetModule& net);
	static void HolePunchingFunc(NetModule& net);

public:
	void InitRoomInfo(SC::packet_room_inside_info* pck);
	void RemovePlayer(SC::packet_remove_player* pck);
	void UpdateMapIndex(SC::packet_update_map_info* pck);
	void UpdatePlayerInfo(SC::packet_update_player_info* pck);
	void UpdateRoomList(SC::packet_room_outside_info* pck);

	void InitPlayerTransform(SC::packet_game_start_success* pck);

public:
	void SetInterface(Scene* scenePtr) { mScenePtr = scenePtr; }
	NetClient* Client() const { return mNetClient.get(); }

	void SetRoomID(int roomID) { mRoomID = roomID; }
	int GetRoomID() const { return mRoomID; }

	void SetIsUpdatedRoomList(bool IsUpdatedRoomList) { mIsUpdatedRoomList = IsUpdatedRoomList; }
	bool GetIsUpdatedRoomList() const { return mIsUpdatedRoomList; }

	char GetPlayerIndex() const { return mPlayerIdx; }
	char GetMapIndex() const { return mMapIdx; }
	char GetAdminIndex() const { return mAdminIdx; }
	bool IsAdmin() const { return mPlayerIdx == mAdminIdx; }

	PlayerList GetPlayersInfo();
	RoomList GetRoomList();

	void CalcClockDelta(uint64_t serverTimeStamp);
	int64_t GetClockDelta() const { return mClockSyncDeltaMs; }
	uint64_t GetServerTimeStamp() const { return mServerTimeStamp; }

	void SetLatency(uint64_t latencyMs) { mLatencyMs = latencyMs; }
	float GetLatency() const { return (float)mLatencyMs / 1000.0f; }

	void SetHolePunchingDone(bool b) { mHolePunchingDone = b; }
	bool IsHolePunchingDone() { return mHolePunchingDone; }
	
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

	RoomList mRoomList;
	std::mutex mRoomListMut;
	std::atomic_bool mIsUpdatedRoomList;

	std::unique_ptr<NetClient> mNetClient;
	std::thread mNetThread;
	std::thread mHolePunchingThread;

	std::atomic_bool mHolePunchingDone;

	std::atomic_uint64_t mServerTimeStamp;
	std::atomic_int64_t mClockSyncDeltaMs;
	std::atomic_uint64_t mLatencyMs;

	IOCP mIOCP;
	Scene* mScenePtr;
};
