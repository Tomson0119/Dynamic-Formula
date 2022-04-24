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

	virtual void Update(float elapsed, BPHandler& physics) override;
	virtual void Reset(BPHandler& physics) override;
	virtual int GetMask(const btCollisionObject& obj) const override;

public:
	void SetGameConstant(std::shared_ptr<InGameServer::GameConstant> constantPtr);
	void SetTransform(const btVector3& pos, const btQuaternion& quat);

	void CreateVehicleRigidBody(btScalar mass, BPHandler& physics, BtCarShape& shape);
	void CreateMissileRigidBody(btScalar mass, BtBoxShape& shape);

	void StopVehicle();
	void ChangeVehicleMaskGroup(int maskGroup, BPHandler& physics);

	void SetDeletionFlag();
	void DisableMissile();

	void SetInvincible();

	void UpdateWorldTransform();
	void UpdateInvincibleDuration(float elapsed);

	void ToggleKeyValue(uint8_t key, bool pressed);
	bool CheckMissileExist() const;

	bool IsNextCheckpoint(int cpIndex);
	void MarkNextCheckpoint(int cpIndex);
	int GetReverseDriveCount(int cpIndex);

	bool NeedUpdate();

	void ClearVehicleComponent();

private:
	void UpdateVehicleComponent(float elapsed);
	void UpdateDriftGauge(float elapsed);
	void UpdateBooster(float elapsed);
	void UpdateSteering(float elapsed);
	void UpdateEngineForce();

	bool UseItem(uint8_t key);
	bool IsItemAvailable();

public:
	void Activate() { mActive = true; }
	void Deactivate() { mActive = false; }
	bool IsActive() const { return mActive; }

	void IncreasePoint(int point) { mPoint += point; }
	void SetCheckpointCount(int count) { mCPPassed.resize(count, false); }

	void ReleaseInvincible() { mInvincible = false; mInvincibleDuration = 0.0f; }
	bool IsInvincible() const { return mInvincible; }
	float GetInvincibleDuration() const { return mInvincibleDuration; }

	int GetCurrentCPIndex() const { return mCurrentCPIndex; }
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
	int mReverseDriveCount;
	int mLapCount;

	float mDriftGauge;

	float mInvincibleDuration;
	bool mInvincible;

	std::atomic_bool mActive;
	std::atomic_int mPoint;
	std::atomic_int mItemCount;
	std::atomic_bool mBoosterToggle;

	std::shared_ptr<InGameServer::GameConstant> mConstantPtr;
};