#include "common.h"
#include "Player.h"
#include "BtShape.h"

Player::Player()
	: mPosition{ 0.0f, 0.0f, 0.0f },
	  Empty{ true }, Color{ -1 }, Ready{ false }, 
	  ID{ -1 }, Name{ }
{
}

void Player::SetPosition(float x, float y, float z)
{
	mPosition = { x, y, z };
}

void Player::CreateVehicleRigidBody(
	btScalar mass,
	btDiscreteDynamicsWorld* physicsWorld, 
	BtCarShape* shape)
{
	if (shape && physicsWorld)
	{
		mVehicleRigidBody.CreateRigidBody(
			physicsWorld, mass,
			shape->GetCollisionShape(), mPosition);

		mVehicleRigidBody.CreateRaycastVehicle(
			physicsWorld, shape->GetExtents(),
			shape->GetWheelInfo());
	}
}