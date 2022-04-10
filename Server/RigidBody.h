#pragma once

#include "BtCollisionShape.h"
#include "BtCompoundShape.h"
#include "InGameServer.h"

class RigidBody
{
public:
	enum class UPDATE_FLAG : uint8_t
	{
		NONE = 0,
		CREATION,
		UPDATE,
		DELETION
	};

public:
	RigidBody();
	virtual ~RigidBody() = default;

	virtual void CreateRigidBody(btScalar mass, btCollisionShape& shape);

	void SetPosition(const btVector3& pos) { mPosition = pos; }
	void SetRotation(const btQuaternion& quat) { mQuaternion = quat; }

	void Update(btDiscreteDynamicsWorld* physicsWorld);
	void UpdateTransformVectors();

	virtual void AppendRigidBody(btDiscreteDynamicsWorld* physicsWorld);
	virtual void UpdateRigidBody();
	virtual void RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld);

public:
	void SetUpdateFlag(UPDATE_FLAG flag) { mFlag = flag; }
	bool ChangeUpdateFlag(UPDATE_FLAG expected, UPDATE_FLAG desired);
	UPDATE_FLAG GetUpdateFlag() const { return mFlag; }
	btRigidBody* GetRigidBody() const { return mRigidBody; }

public:
	const btVector3& GetPosition() const { return mPosition; }
	const btQuaternion& GetQuaternion() const { return mQuaternion; }
	const btVector3& GetLinearVelocity() const { return mLinearVelocity; }
	const btVector3& GetAngularVelocity() const { return mAngularVelocity; }

protected:
	btRigidBody* mRigidBody;

	btVector3 mPosition;
	btQuaternion mQuaternion;

	btVector3 mLinearVelocity;
	btVector3 mAngularVelocity;

	std::atomic<UPDATE_FLAG> mFlag;
};

class MissileRigidBody : public RigidBody
{
public:
	MissileRigidBody();
	virtual ~MissileRigidBody() = default;

public:
	virtual void AppendRigidBody(btDiscreteDynamicsWorld* physicsWorld) override;
	virtual void UpdateRigidBody() override;

	void SetVehicleAndConstantPtr(
		class VehicleRigidBody* vehiclePtr, 
		std::shared_ptr<InGameServer::BulletConstant> constantPtr);

	void SetMissileComponents(
		const btVector3& position, 
		const btVector3& forward,
		const btQuaternion& rotation,
		const btVector3& gravity,
		float speed);

private:
	VehicleRigidBody* mVehiclePtr;
	std::shared_ptr<InGameServer::BulletConstant> mConstantPtr;
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
		btDiscreteDynamicsWorld* physicsWorld,
		const btVector3& bodyExtents, 
		const BtCarShape::WheelInfo& wheelInfo);
	
	void AddWheel(const btVector3& bodyExtents, const BtCarShape::WheelInfo& wheelInfo);
	void StoreWorldTransform(btTransform& transform);

public:
	virtual void AppendRigidBody(btDiscreteDynamicsWorld* physicsWorld) override;
	virtual void UpdateRigidBody() override;
	virtual void RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld) override;

public:
	btRaycastVehicle* GetVehicle() const { return mVehicle.get(); }
	const Tuning& GetTuning() const { return mTuning; }
	VehicleComponent& GetComponent() { return mComponent; }
	btVector3 GetForwardVector() const { return mVehicle->getForwardVector(); }
	float GetCurrentSpeed() const { return mComponent.CurrentSpeed; }

private:
	Tuning mTuning;
	VehicleComponent mComponent;
	std::unique_ptr<btVehicleRaycaster> mVehicleRayCaster;
	std::unique_ptr<btRaycastVehicle> mVehicle;
};

class MapRigidBody
{
public:
	MapRigidBody() = default;
	~MapRigidBody() = default;
	
	void CreateTerrainRigidBody(BtTerrainShape* shape);
	void CreateStaticRigidBodies(std::string_view filename,	btCollisionShape* shape);

	void UpdateRigidBodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld);
	void RemoveRigidBodies(btDiscreteDynamicsWorld* physicsWorld);

private:
	std::deque<RigidBody> mStaticRigidBodies;
};