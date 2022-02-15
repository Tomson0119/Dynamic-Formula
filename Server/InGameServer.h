#pragma once

#include "BtShape.h"
#include "Timer.h"

class LoginServer;
class WaitRoom;
class GameWorld;

class InGameServer
{
	using RoomList = std::array<std::unique_ptr<WaitRoom>, MAX_ROOM_SIZE>;
	using WorldList = std::array<std::unique_ptr<GameWorld>, MAX_ROOM_SIZE>;
public:
	InGameServer();
	~InGameServer() = default;

	void Init(LoginServer* loginPtr, RoomList& roomList);
	void PrepareToStartGame(int roomID);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);
	void HandleKeyInput(int id, uint8_t key, bool pressed);

	void RemovePlayer(int roomID, int hostID);

	void AddPhysicsTimerEvent(int roomID);
	void RunPhysicsSimulation(int roomID, float timeStep);
	void PostPhysicsOperation(int roomID, float timeStep);

private:
	LoginServer* mLoginPtr;
	static WorldList msWorlds;

	Timer mTimer;
	
	std::unique_ptr<BtCarShape> mBtCarShape;
	std::unique_ptr<BtBoxShape> mMissileShape;

	std::array<std::unique_ptr<BtTerrainShape>, 2> mTerrainShapes;
	std::vector<std::unique_ptr<BtBoxShape>> mObjRigidBodies;

	const btVector3 mStartPosition = { 500.0f, 10.0f, 500.0f };
	const int mDuration = 1000;
};