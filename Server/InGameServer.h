#pragma once

#include "BPHandler.h"
#include "Player.h"

class LoginServer;

struct GameWorld
{
	BPHandler Physics;
	std::array<Player, MAX_ROOM_CAPACITY> PlayerList;
};

class InGameServer
{
	using WorldList = std::array<std::unique_ptr<GameWorld>, MAX_ROOM_SIZE>;
public:
	InGameServer();
	~InGameServer() = default;

	void Init(LoginServer* loginPtr);
	void PrepareToStartGame(int roomID, const std::array<int, MAX_ROOM_CAPACITY>& ids);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);
	void HandleKeyInput(int id, uint8_t key, bool pressed);


private:
	LoginServer* mLoginPtr;

	static WorldList msWorlds;
};