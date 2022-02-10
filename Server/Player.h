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

public:
	bool Empty;
	char Color;
	bool Ready;
	int ID;
	char Name[MAX_NAME_SIZE];

private:
	btVector3 mPosition;
	VehicleRigidBody mVehicleRigidBody;
};