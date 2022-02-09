#pragma once

#include "BPHandler.h"
#include "BtShape.h"

class LoginServer;
class WaitRoom;
class Player;

struct GameWorld
{
	BPHandler Physics;
	uint8_t MapIdx;
	std::array<Player*, MAX_ROOM_CAPACITY> PlayerList;
};

class InGameServer
{
	using ShapeList = std::vector<std::unique_ptr<BtShapeBase>>;
	using RoomList = std::array<std::unique_ptr<WaitRoom>, MAX_ROOM_SIZE>;
	using WorldList = std::array<std::unique_ptr<GameWorld>, MAX_ROOM_SIZE>;
public:
	InGameServer();
	~InGameServer() = default;

	void Init(LoginServer* loginPtr, RoomList& roomList);
	void PrepareToStartGame(int roomID);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);
	void HandleKeyInput(int id, uint8_t key, bool pressed);

public:
	void SendGameStartSuccess(int roomID);

private:
	void SendToAllPlayer(std::byte* pck, int size, int roomID, int ignore=-1, bool instSend=true);

private:
	LoginServer* mLoginPtr;
	static WorldList msWorlds;
	
	ShapeList mBtShapes;
	
	const btVector3 mStartPosition = { 500.0f, 10.0f, 500.0f };
};