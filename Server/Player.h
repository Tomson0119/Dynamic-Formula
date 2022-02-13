#pragma once

#include "RigidBody.h"

class BtCarShape;

class Player
{
public:
	Player();
	~Player() = default;

	void SetPosition(float x, float y, float z);
	const btVector3& GetPosition() const { return mPosition; }

	void CreateVehicleRigidBody(
		btScalar mass,
		btDiscreteDynamicsWorld* physicsWorld, 
		BtCarShape* shape);

	void UpdatePlayerRigidBody(btDiscreteDynamicsWorld* physicsWorld);
	void SetDeletionFlag() { mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION); }

	void RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld);

public:
	std::atomic_bool Empty;
	std::atomic_bool Ready;
	std::atomic_int ID;
	char Color;
	char Name[MAX_NAME_SIZE];

private:
	btVector3 mPosition;
	VehicleRigidBody mVehicleRigidBody;
};