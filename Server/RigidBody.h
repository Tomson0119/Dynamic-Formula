#pragma once

#include "BtShape.h"

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

	virtual void CreateRigidBody(
		btScalar mass,
		btCollisionShape* shape,
		const btVector3& position,
		const btVector3& offset = btVector3{ 0.0f,0.0f,0.0f });

	void Update(btDiscreteDynamicsWorld* physicsWorld);

	virtual void AppendRigidBody(btDiscreteDynamicsWorld* physicsWorld);
	virtual void UpdateRigidBody();
	virtual void RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld);

public:
	void SetUpdateFlag(UPDATE_FLAG flag) { mFlag = flag; }
	bool ChangeUpdateFlag(UPDATE_FLAG expected, UPDATE_FLAG desired);
	UPDATE_FLAG GetUpdateFlag() const { return mFlag; }

	btRigidBody* GetRigidBody() const { return mRigidBody; }

protected:
	btRigidBody* mRigidBody;
	std::atomic<UPDATE_FLAG> mFlag;
};

class VehicleRigidBody : public RigidBody
{
	using Tuning = btRaycastVehicle::btVehicleTuning;
	
public:
	struct VehicleComponent
	{
		float BoosterLeft{};
		float EngineForce{};
		float VehicleSteering{};
		float CurrentSpeed{};
		float FrictionSlip{};
		float MaxSpeed{};
	};

public:
	VehicleRigidBody() = default;
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

	void UpdateAllRigidBody(float elapsed, btDiscreteDynamicsWorld* physicsWorld);

private:
	std::deque<RigidBody> mStaticRigidBodies;
};