#pragma once

#include "BtShape.h"

class RigidBody
{
public:
	RigidBody();
	virtual ~RigidBody() = default;

	virtual void CreateRigidBody(
		btDiscreteDynamicsWorld* physicsWorld,
		btScalar mass,
		btCollisionShape* shape,
		const btVector3& position,
		const btVector3& offset = btVector3{ 0.0f,0.0f,0.0f });

	btRigidBody* GetRigidBody() const { return mRigidBody; }

protected:
	btRigidBody* mRigidBody;
};

class VehicleRigidBody : public RigidBody
{
	using Tuning = btRaycastVehicle::btVehicleTuning;
public:
	VehicleRigidBody() = default;
	virtual ~VehicleRigidBody() = default;

	void CreateRaycastVehicle(
		btDiscreteDynamicsWorld* physicsWorld,
		const btVector3& bodyExtents, 
		const BtCarShape::WheelInfo& wheelInfo);
	void AddWheel(const btVector3& bodyExtents, const BtCarShape::WheelInfo& wheelInfo);

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
	
	void CreateTerrainRigidBody(btDiscreteDynamicsWorld* physics, BtTerrainShape* shape);

	void CreateStaticRigidBodies(
		std::string_view filename,
		btDiscreteDynamicsWorld* physicsWorld,
		btCollisionShape* shape);

private:
	std::vector<RigidBody> mStaticRigidBodies;
};