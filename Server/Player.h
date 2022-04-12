#pragma once

#include "GameObject.h"
#include "RigidBody.h"
#include "InGameServer.h"

class BtCarShape;

class Player : public GameObject
{
public:
	Player();
	virtual ~Player() = default;

	virtual void UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld) override;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) override;

	void SetBulletConstant(std::shared_ptr<InGameServer::BulletConstant> constantPtr);
	void SetPosition(const btVector3& pos);
	void SetRotation(const btQuaternion& quat);

	void CreateVehicleRigidBody(
		btScalar mass,
		btDiscreteDynamicsWorld* physicsWorld, 
		BtCarShape* shape);

	void CreateMissileRigidBody(btScalar mass, BtBoxShape* shape);
	void SetDeletionFlag();

	void UpdateWorldTransform();
	void ToggleKeyValue(uint8_t key, bool pressed);
	bool CheckMissileExist() const;

private:
	void ClearVehicleComponent();

	void UpdateVehicleComponent(float elapsed);
	void UpdateDriftGauge(float elapsed);
	void UpdateBooster(float elapsed);
	void UpdateSteering(float elapsed);
	void UpdateEngineForce();

	bool UseItem(uint8_t key);
	bool IsItemAvailable();

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
	MissileRigidBody mMissileRigidBody;

	std::atomic_bool mBoosterToggle;
	
	float mDriftGauge;
	std::atomic_int mItemCount;

	std::shared_ptr<InGameServer::BulletConstant> mConstantPtr;
};