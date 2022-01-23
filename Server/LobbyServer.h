#pragma once

class LoginServer;
class InGameRoom;
class Client;

class LobbyServer
{
	using RoomList = std::array<std::unique_ptr<InGameRoom>, MAX_ROOM_SIZE>;
public:
	LobbyServer();
	~LobbyServer();

	void Init(LoginServer* ptr);

	void AcceptLogin(const char* name, int id);
	void Logout(int id);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);
	void TryRemovePlayer(int roomID, int hostID);

	void SendRoomInfoToLobbyPlayers(int roomID, bool instSend = true);
	void SendExistingRoomList(int id);

	// TEST
	void PrintRoomList();

private:
	std::atomic_int mRoomCount;
	std::atomic_int mLobbyPlayerCount;

	static RoomList gRooms;

	LoginServer* mLoginPtr;
};