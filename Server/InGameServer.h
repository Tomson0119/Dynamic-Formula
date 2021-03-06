#pragma once

#include "BtCollisionShape.h"
#include "BtCompoundShape.h"
#include "Timer.h"
#include "WaitRoom.h"
#include "GameWorld.h"

class LoginServer;

struct GameConstant
{
	const float MaxBoosterTime = 5.0f;

	const float MaxEngineForce = 3000.f;
	const float BoosterEngineForce = 7000.f;

	const float SteeringIncrement = 5.0f;
	const float SteeringClamp = 0.15f;

	const float FrontWheelDriftFriction = 1.5f;
	const float	RearWheelDriftFriction = 0.0f;
	const float WheelDefaultFriction = 5.0f;
	const float MinSpeedForDrift = 50.0f;

	const float DefaultMaxSpeed = 200.0f;
	const float BoostedMaxSpeed = 250.0f;

	const float DefaultBreakingForce = 30.0f;

	const float MissileSpeed = 50.0f;
	const btVector3 MissileOffset = { 0.0f, 0.5f, 0.0f };
	const float MissileForwardMag = 5.0f;
	const btVector3 MissileGravity = { 0.0f, 0.0f, 0.0f };

	const int MissileHitPoint = 1000;
	const int LapFinishPoint = 2500;

	const float InvincibleDuration = 3.0f;
	const float SpawnInterval = InvincibleDuration / 2.0f;

	const btVector3 StartPosition = { -190.7f, 1.0f, 250.0f };
	const btQuaternion StartRotation = { 0.0f, 0.707107f, 0.0f, -0.707107f };

	const std::chrono::seconds CountdownTime = 4s; // 4초 후 시작
	const std::chrono::seconds GameRunningTime = 300s; // 30초 TEST
};

class InGameServer
{
	using RoomList = std::array<std::unique_ptr<WaitRoom>, MAX_ROOM_SIZE>;
	using WorldList = std::array<std::unique_ptr<GameWorld>, MAX_ROOM_SIZE>;
public:
	InGameServer();
	~InGameServer() = default;

	void Init(LoginServer* loginPtr, RoomList& roomList);
	void PrepareToStartGame(int roomID, char mapIdx);

	bool ProcessPacket(std::byte* packet, const CS::PCK_TYPE& type, int id, int bytes);

	void StartMatch(int roomID);
	void RemovePlayer(int roomID, int hostID);

	void AddTimerEvent(int roomID, EVENT_TYPE type, int duration);
	void RunPhysicsSimulation(int roomID);

	void PostPhysicsOperation(int roomID);

public:
	bool IsWorldActive(int roomID) { return msWorlds[roomID]->IsActive(); }

private:
	LoginServer* mLoginPtr;
	WorldList msWorlds;

	TimerQueue mTimerQueue;
	
	std::unique_ptr<BtCarShape> mBtCarShape;
	std::unique_ptr<BtMissileShape> mMissileShape;
	std::array<std::unique_ptr<BtMapShape>, 2> mMapShapes;
	std::unique_ptr<CheckpointShape> mCheckpointShape;

	std::shared_ptr<GameConstant> mGameConstants;

	const btVector3 mOffset = { 10.0f, 0.0f, 4.1f };
	const int mPhysicsDurationMs = 8;
};