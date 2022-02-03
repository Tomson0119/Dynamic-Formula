#pragma once

class LoginServer;
class InGameRoom;
class Client;

class LobbyServer
{
	using RoomList = std::array<std::unique_ptr<InGameRoom>, MAX_ROOM_SIZE>;
public:
	LobbyServer();
	~LobbyServer() = default;

	void Init(LoginServer* ptr);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	void AcceptEnterRoom(int roomID, int hostID);
	bool TryAddPlayer(int roomID, int hostID);
	void TryRemovePlayer(int roomID, int hostID);

	void SendRoomInfoToLobbyPlayers(int roomID, bool instSend = true);
	void SendExistingRoomList(int id);

	// TEST
	void PrintRoomList();

public:
	void IncreasePlayerCount() { mLobbyPlayerCount.fetch_add(1); }
	void DecreasePlayerCount() { mLobbyPlayerCount.fetch_sub(1); }

private:
	std::atomic_int mRoomCount;
	std::atomic_int mLobbyPlayerCount;

	static RoomList gRooms;

	LoginServer* mLoginPtr;
};