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
	struct BulletConstant
	{
		const float MaxBoosterTime = 5.0f;

		const float MaxEngineForce = 8000.f;
		const float MaxBackwardEngineForce = 10000.0f;
		const float BoosterEngineForce = 300000.f;

		const float SteeringIncrement = 8.0f;
		const float SteeringClamp = 0.5f;

		const float	WheelDriftFriction = 4.0f;
		const float WheelDefaultFriction = 25.0f;
		const float MinSpeedForDrift = 100.0f;

		const float DefaultMaxSpeed = 1000.0f;
		const float BoostedMaxSpeed = 1500.0f;

		const float DefaultBreakingForce = 10.0f;
		const float MaxBreakingForce = 150.0f;
		const float SubBreakingForce = 100.0f;

		const float MissileForwardOffset = 15.0f;
		const float MissileSpeed = 1000.0f;
		const btVector3 MissileGravity = { 0.0f, 0.0f, 0.0f };
	};

public:
	InGameServer();
	~InGameServer() = default;

	void Init(LoginServer* loginPtr, RoomList& roomList);
	void PrepareToStartGame(int roomID);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

	void StartMatch(int roomID);
	void RemovePlayer(int roomID, int hostID);

	void AddTimerEvent(int roomID, EVENT_TYPE type, int duration);
	void RunPhysicsSimulation(int roomID);

	void PostPhysicsOperation(int roomID);

private:
	LoginServer* mLoginPtr;
	static WorldList msWorlds;

	TimerQueue mTimerQueue;
	
	std::unique_ptr<BtCarShape> mBtCarShape;
	std::unique_ptr<BtBoxShape> mMissileShape;

	std::array<std::unique_ptr<BtTerrainShape>, 2> mTerrainShapes;
	std::vector<std::unique_ptr<BtBoxShape>> mObjRigidBodies;

	std::shared_ptr<BulletConstant> mBulletConstants;

	const btVector3 mStartPosition = { 500.0f, 10.0f, 500.0f };
	const btVector3 mOffset = { 20.0f, 0.0f, 0.0f };

	const int mPhysicsDuration = 16;
};