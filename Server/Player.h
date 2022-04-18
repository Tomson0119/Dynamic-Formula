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

	virtual void Update(float elapsed, btDiscreteDynamicsWorld* physicsWorld) override;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) override;
	virtual OBJ_TAG GetTag(const btCollisionObject& obj) const override;

public:
	void SetGameConstant(std::shared_ptr<InGameServer::GameConstant> constantPtr);
	void SetPosition(const btVector3& pos);
	void SetRotation(const btQuaternion& quat);

	void CreateVehicleRigidBody(
		btScalar mass,
		btDiscreteDynamicsWorld* physicsWorld, 
		BtCarShape* shape);

	void CreateMissileRigidBody(btScalar mass, BtBoxShape* shape);
	void SetDeletionFlag();
	void SetMissileDeletionFlag();

	void SetInvincible();

	void UpdateWorldTransform();
	void ToggleKeyValue(uint8_t key, bool pressed);
	bool CheckMissileExist() const;

	void HandleCheckpointCollision(int cpIndex);

private:
	void ClearVehicleComponent();

	void UpdateVehicleComponent(float elapsed);
	void UpdateInvincibleDuration(float elapsed);
	void UpdateDriftGauge(float elapsed);
	void UpdateBooster(float elapsed);
	void UpdateSteering(float elapsed);
	void UpdateEngineForce();

	bool UseItem(uint8_t key);
	bool IsItemAvailable();

public:
	void IncreasePoint(int point) { mPoint += point; }
	void SetCheckpointCount(int count) { mCPPassed.resize(count, false); }

	bool IsInvincible() const { return mInvincible; }
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
	std::vector<bool> mCPPassed;

	VehicleRigidBody mVehicleRigidBody;
	MissileRigidBody mMissileRigidBody;

	int mCurrentCPIndex;
	int mLapCount;
	float mDriftGauge;
	float mInvincibleDuration;
	bool mInvincible;

	std::atomic_int mPoint;
	std::atomic_int mItemCount;
	std::atomic_bool mBoosterToggle;

	std::shared_ptr<InGameServer::GameConstant> mConstantPtr;
};