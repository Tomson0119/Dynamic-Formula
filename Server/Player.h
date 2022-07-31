#pragma once

#include "GameObject.h"
#include "RigidBody.h"

class BtCarShape;
struct GameConstant;

class Player : public GameObject
{
public:
	Player();
	virtual ~Player() = default;

	virtual void Update(float elapsed, BPHandler& physics) override;
	virtual void Reset(BPHandler* physics) override;
	virtual int GetMask(const btCollisionObject& obj) const override;

public:
	void SetGameConstant(std::shared_ptr<GameConstant> constantPtr);
	void SetTransform(const btVector3& pos, const btQuaternion& quat);

	void CreateVehicleRigidBody(btScalar mass, BPHandler& physics, BtCarShape& shape);
	void CreateMissileRigidBody(btScalar mass, BtCompoundShape& shape);

	void StopVehicle();
	void ChangeVehicleMaskGroup(int maskGroup, BPHandler& physics);

	void SetRemoveFlag();
	void DisableMissile();

	void SetInvincible(float duration);

	void UpdateWorldTransform();
	void UpdateInvincibleDuration(float elapsed);

	bool ToggleKeyValue(uint8_t key, bool pressed);
	bool CheckMissileExist() const;
	bool MissileTimeOver() const;

	bool IsNextCheckpoint(int cpIndex);
	void MarkNextCheckpoint(int cpIndex);
	int GetReverseDriveCount(int cpIndex);

	bool NeedUpdate();
	void ClearVehicleComponent();
	void ClearAllGameInfo();

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

	void ResetReverseCount() { mReverseDriveCount = 0; }
	void IncreaseHitCount() { mHitCount += 1; }
	void IncreaseLapCount() { mLapCount += 1; }
	void IncreasePoint(int point) { mPoint += point; }
	void SetCheckpointCount(int count) { mCPPassed.resize(count, false); }

	void ReleaseInvincible() { mInvincible = false; mInvincibleDuration = 0.0f; }
	bool IsInvincible() const { return mInvincible; }
	float GetInvincibleDuration() const { return mInvincibleDuration; }	

	int GetItemCount() const { return mItemCount; }
	int GetHitCount() const { return mHitCount; }
	int GetLapCount() const { return mLapCount; }
	int GetPoint() const { return mPoint; }
	int GetCurrentCPIndex() const { return mCurrentCPIndex; }
	
	float GetCurrentSpeed() const { return mVehicleRigidBody.GetCurrentSpeed(); }
	float GetDriftGauge() const { return mDriftGauge; }

	void ResetManualRespawnFlag() { mManualRespawn = false; }
	bool NeedManualRespawn() const { return mManualRespawn; }

	void ResetItemFlag() { mItemIncreased = false; }
	bool ItemIncreased() const { return mItemIncreased; }

	const VehicleRigidBody& GetVehicleRigidBody() const { return mVehicleRigidBody; }
	const RigidBody& GetMissileRigidBody() const { return mMissileRigidBody; }
	
	void SetMissileLaunchFlag(bool flag) { mMissileRigidBody.SetLaunchFlag(flag); }
	bool GetMissileLaunchFlag() const { return mMissileRigidBody.GetLaunchFlag(); }

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
	int mHitCount;
	int mPoint;
	int mReverseDriveCount;

	float mDriftGauge;

	float mInvincibleDuration;
	bool mInvincible;
	bool mItemIncreased;

	std::atomic_bool mManualRespawn;
	std::atomic_bool mActive;
	std::atomic_int mItemCount;
	std::atomic_bool mBoosterToggle;

	std::shared_ptr<GameConstant> mConstantPtr;
};