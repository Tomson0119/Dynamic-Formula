#pragma once

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	int ID;
	char Name[MAX_NAME_SIZE];
};

class LoginServer;

class WaitRoom
{
	using PlayerList = std::array<PlayerInfo, MAX_ROOM_CAPACITY>;
public:
	WaitRoom(int id);
	~WaitRoom() = default;

	bool OpenRoom();
	bool CloseRoom();

	int FindAvaliableSpace();

	bool AddPlayer(int hostID);
	bool RemovePlayer(int hostID);

	void SwitchMap(int hostID);
	bool TryGameStart(int hostID);

	std::array<int, MAX_ROOM_CAPACITY> GetPlayersID();

public:
	void IncreasePlayerCount();
	void DecreasePlayerCount();

	char GetPlayerCount() const { return mPlayerCount; }
	char GetMapIndex() const { return mMapIndex; }
	char GetAdminIndex() const { return mAdminIndex; }

	bool Empty() const { return (mPlayerCount == 0); }
	bool Available() const { return (mState == ROOM_STAT::AVAILABLE); }
	bool Full() const { return (mState == ROOM_STAT::ROOM_IS_FULL); }
	bool Closed() const { return (mState == ROOM_STAT::ROOM_IS_CLOSED); }
	bool GameRunning() const { return (mState == ROOM_STAT::GAME_STARTED); }

	ROOM_STAT GetRoomState() const { return mState; }

public:
	void SendGameStartResult(int hostID, bool result, bool instSend = true);

	void SendUpdatePlayerInfoToAll(int target, int ignore=-1, bool instSend=true);
	void SendRemovePlayerInfoToAll(int target, bool instSend=true);
	void SendUpdateMapInfoToAll(int ignore=-1, bool instSend=true);

	void SendRoomInsideInfo(int id, bool instSend=true);
	void SendRoomOutsideInfo(int id, bool instSend=true);

private:
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);

private:
	int mID;
	std::atomic_char mPlayerCount;	
	std::atomic_char mMapIndex;
	std::atomic_char mAdminIndex;

	std::atomic<ROOM_STAT> mState;

	static PlayerList msPlayers;
};