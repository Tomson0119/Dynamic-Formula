#pragma once

#include "RigidBody.h"
#include "InGameServer.h"

class BtCarShape;

class Player
{
public:
	Player();
	~Player() = default;

	void SetBulletConstant(std::shared_ptr<InGameServer::BulletConstant> constantPtr);
	
	void SetPosition(const btVector3& pos);
	void SetRotation(const btQuaternion& quat);

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
	
	bool CheckMissileExist() const;

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
	MissileRigidBody mMissileRigidBody;

	std::shared_ptr<InGameServer::BulletConstant> mConstantPtr;
};