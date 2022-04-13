#pragma once

#include "BtCollisionShape.h"
#include "BtCompoundShape.h"
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

		const float MaxEngineForce = 1000.f;
		const float BoosterEngineForce = 3000.f;

		const float SteeringIncrement = 5.0f;
		const float SteeringClamp = 0.15f;

		const float	WheelDriftFriction = 5.0f;
		const float WheelDefaultFriction = 10.0f;
		const float MinSpeedForDrift = 100.0f;

		const float DefaultMaxSpeed = 350.0f;
		const float BoostedMaxSpeed = 400.0f;

		const float DefaultBreakingForce = 10.0f;

		const float MissileSpeed = 10.0f;
		const btVector3 MissileOffset = { 0.0f, 2.0f, 0.0f };
		const float MissileForwardMag = 10.0f;
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
	std::unique_ptr<BtMapShape> mMapShape;
	std::unique_ptr<CheckpointShape> mCheckpointShape;

	std::shared_ptr<BulletConstant> mBulletConstants;

	const btVector3 mStartPosition = { -306.5f, 1.0f, 253.7f };
	const btQuaternion mStartRotation = { 0.0f, 0.707107f, 0.0f, -0.707107f };
	const btVector3 mOffset = { 20.0f, 0.0f, 0.0f };

	const int mPhysicsDuration = 16;
};