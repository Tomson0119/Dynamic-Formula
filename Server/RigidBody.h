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

	virtual void UpdateRigidBody(btDiscreteDynamicsWorld* physicsWorld);

	void SetUpdateFlag(UPDATE_FLAG flag) { mFlag = flag; }
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
	VehicleRigidBody() = default;
	virtual ~VehicleRigidBody() = default;

	virtual void UpdateRigidBody(btDiscreteDynamicsWorld* physicsWorld);

public:
	void CreateRaycastVehicle(
		btDiscreteDynamicsWorld* physicsWorld,
		const btVector3& bodyExtents, 
		const BtCarShape::WheelInfo& wheelInfo);
	
	void AddWheel(const btVector3& bodyExtents, const BtCarShape::WheelInfo& wheelInfo);

	void UpdateVehicleComponent(btDiscreteDynamicsWorld* physicsWorld);

public:
	btRaycastVehicle* GetVehicle() const { return mVehicle.get(); }
	const Tuning& GetTuning() const { return mTuning; }

private:
	Tuning mTuning;
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

	void UpdateRigidBody(btDiscreteDynamicsWorld* physicsWorld);

private:
	std::deque<RigidBody> mStaticRigidBodies;
};