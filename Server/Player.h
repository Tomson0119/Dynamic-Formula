#pragma once

#include "RigidBody.h"
#include "InGameServer.h"

class BtCarShape;

class Player
{
public:
	Player();
	~Player() = default;

	void SetVehicleConstant(std::shared_ptr<InGameServer::VehicleConstant> constantPtr);
	void SetPosition(float x, float y, float z);

	void CreateVehicleRigidBody(
		btScalar mass,
		btDiscreteDynamicsWorld* physicsWorld, 
		BtCarShape* shape);

	void CreateMissileRigidBody(btScalar mass, BtBoxShape* shape);

	void UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld);
	void SetDeletionFlag();

	void ResetPlayer(btDiscreteDynamicsWorld* physicsWorld);
	void UpdateWorldTransform();

	void ClearVehicleComponent();

	void UpdateDiftGauge(float elapsed);
	void UpdateVehicleComponent(float elapsed);
	void UpdateSteering(float elapsed);
	void UpdateEngineForce();

	bool CheckDriftGauge();
	void ToggleKeyValue(uint8_t key, bool pressed);

public:
	const VehicleRigidBody& GetVehicleRigidBody() const { return mVehicleRigidBody; }
	const RigidBody& GetMissileRigidBody() const { return mMissileRigidBody; }

public:
	std::atomic_bool Empty;
	std::atomic_bool Ready;
	std::atomic_int ID;
	char Color;
	char Name[MAX_NAME_SIZE];

	std::atomic_bool LoadDone;

private:
	std::map<int, std::atomic_bool> mKeyMap;

	VehicleRigidBody mVehicleRigidBody;
	RigidBody mMissileRigidBody;
	std::shared_ptr<InGameServer::VehicleConstant> mConstantPtr;

	inline static const float msForwardOffset = 15.0f;
};