#pragma once

#include "BtCollisionShape.h"
#include "BtCompoundShape.h"
#include "InGameServer.h"

class GameObject;
class BPHandler;

class RigidBody
{
public:
	enum class UPDATE_FLAG : uint8_t
	{
		NONE = 0,
		CREATE,
		UPDATE,
		REMOVE,
		CHANGE_MASK,
	};

public:
	RigidBody();
	virtual ~RigidBody();

	void SetMaskBits(int maskGroup, int mask);
	void SetNoResponseCollision();
	void CreateRigidBody(btScalar mass, btCollisionShape& shape, GameObject* objPtr);

	void SetTransform(const btVector3& pos, const btQuaternion& quat);
	void SetLinearVelocity(const btVector3& vec);
	void SetAngularVelocity(const btVector3& vel);

	void Update(BPHandler& physics);
	void UpdateTransformVectors();

	virtual void AppendRigidBody(BPHandler& physics);
	virtual void UpdateRigidBody();
	virtual void RemoveRigidBody(BPHandler& physics);

public:
	void SetUpdateFlag(UPDATE_FLAG flag) { mFlag = flag; }
	bool ChangeUpdateFlag(UPDATE_FLAG expected, UPDATE_FLAG desired);
	UPDATE_FLAG GetUpdateFlag() const { return mFlag; }
	btRigidBody* GetRigidBody() const { return mRigidBody; }

public:
	const btVector3& GetPosition() const { return mPosition; }
	const btQuaternion& GetRotation() const { return mQuaternion; }
	const btVector3& GetLinearVelocity() const { return mLinearVelocity; }
	const btVector3& GetAngularVelocity() const { return mAngularVelocity; }

protected:
	btRigidBody* mRigidBody;

	btVector3 mPosition;
	btQuaternion mQuaternion;

	btVector3 mLinearVelocity;
	btVector3 mAngularVelocity;

	int mMaskGroup;
	int mMask;
	
	std::atomic<UPDATE_FLAG> mFlag;
};

class MissileRigidBody : public RigidBody
{
public:
	MissileRigidBody();
	virtual ~MissileRigidBody() = default;

	void SetGameConstantPtr(
		class VehicleRigidBody* vehiclePtr, 
		std::shared_ptr<InGameServer::GameConstant> constantPtr);

public:
	virtual void AppendRigidBody(BPHandler& physics) override;
	virtual void UpdateRigidBody() override;
	virtual void RemoveRigidBody(BPHandler& physics) override;

public:
	void Activate() { mActive = true; }
	void Deactivate() { mActive = false; }
	bool IsActive() const { return mActive; }

private:
	void SetMissileComponents();

private:
	bool mActive;
	VehicleRigidBody* mVehiclePtr;
	std::shared_ptr<InGameServer::GameConstant> mConstantPtr;
};

class CustomVehicleRaycaster : public btVehicleRaycaster
{
public:
	CustomVehicleRaycaster(btDynamicsWorld* dynamicsWorld);
	virtual ~CustomVehicleRaycaster() = default;
	virtual void* castRay(const btVector3& from, const btVector3& to, btVehicleRaycasterResult& result) override;
	
	void SetMaskBits(int maskGroup, int mask);

private:
	int mMaskGroup;
	int mMask;
	btDynamicsWorld* mDynamicsWorld;
};

class VehicleRigidBody : public RigidBody
{
	using Tuning = btRaycastVehicle::btVehicleTuning;
	
public:
	struct VehicleComponent
	{
		float BoosterTimeLeft{};
		float EngineForce{};
		float BreakingForce{};
		float VehicleSteering{};
		float CurrentSpeed{};
		float FrontFrictionSlip{};
		float BackFrictionSlip{};
		float MaxSpeed{};
	};

public:
	VehicleRigidBody();
	virtual ~VehicleRigidBody() = default;

	void CreateRaycastVehicle(
		BPHandler& physics,
		const btVector3& bodyExtents, 
		const BtCarShape::WheelInfo& wheelInfo);
	
	void AddWheel(const btVector3& bodyExtents, const BtCarShape::WheelInfo& wheelInfo);
	void StoreWorldTransform(btTransform& transform);

public:
	virtual void AppendRigidBody(BPHandler& physics) override;
	virtual void UpdateRigidBody() override;
	virtual void RemoveRigidBody(BPHandler& physics) override;

public:
	btRaycastVehicle* GetVehicle() const { return mVehicle.get(); }
	const Tuning& GetTuning() const { return mTuning; }
	VehicleComponent& GetComponent() { return mComponent; }
	btVector3 GetForwardVector() const { return mVehicle->getForwardVector(); }
	float GetCurrentSpeed() const { return mComponent.CurrentSpeed; }

private:
	Tuning mTuning;
	VehicleComponent mComponent;
	std::unique_ptr<CustomVehicleRaycaster> mVehicleRayCaster;
	std::unique_ptr<btRaycastVehicle> mVehicle;
};