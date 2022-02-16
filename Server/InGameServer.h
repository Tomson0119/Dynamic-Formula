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
	struct VehicleConstant
	{
		const float MaxBoosterTime = 5.0f;
		const float MaxEngineForce = 8000.f;
		const float BoosterEngineForce = 300000.f;
		const float SteeringIncrement = 0.01f;
		const float SteeringClamp = 0.5f;
		const float	FrontWheelDriftFriction = 5.0f;
		const float BackWheelDriftFriction = 3.9f;
	};

public:
	InGameServer();
	~InGameServer() = default;

	void Init(LoginServer* loginPtr, RoomList& roomList);
	void PrepareToStartGame(int roomID);

	bool ProcessPacket(std::byte* packet, char type, int id, int bytes);

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

	std::shared_ptr<VehicleConstant> mVehicleConstants;

	const btVector3 mStartPosition = { 500.0f, 10.0f, 500.0f };
	const int mDurationMs = 16;
};