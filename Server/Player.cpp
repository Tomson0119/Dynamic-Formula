#include "common.h"
#include "Player.h"
#include "BtShape.h"
#include "RigidBody.h"

Player::Player()
	: mPosition{ 0.0f, 0.0f, 0.0f },
	  Empty{ true }, Color{ -1 }, Ready{ false }, 
	  ID{ -1 }, Name{ }
{
	mKeyMap[VK_UP]	   = false;
	mKeyMap[VK_DOWN]   = false;
	mKeyMap[VK_LEFT]   = false;
	mKeyMap[VK_RIGHT]  = false;
	mKeyMap[VK_LSHIFT] = false;
}

void Player::SetPosition(float x, float y, float z)
{
	mPosition = { x, y, z };
}

void Player::SetVehicleConstant(std::shared_ptr<InGameServer::VehicleConstant> constantPtr)
{
	mConstantPtr = constantPtr;
}

void Player::CreateVehicleRigidBody(
	btScalar mass,
	btDiscreteDynamicsWorld* physicsWorld, 
	BtCarShape* shape)
{
	if (shape && physicsWorld)
	{
		mVehicleRigidBody.CreateRigidBody(
			mass,
			shape->GetCollisionShape(), mPosition);

		mVehicleRigidBody.CreateRaycastVehicle(
			physicsWorld, shape->GetExtents(),
			shape->GetWheelInfo());

		mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
	}
}

void Player::UpdatePlayerRigidBody(float elapsed, btDiscreteDynamicsWorld* physicsWorld)
{
	UpdateVehicleComponent(elapsed);
	mVehicleRigidBody.Update(physicsWorld);
}

void Player::RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld)
{
	mVehicleRigidBody.RemoveRigidBody(physicsWorld);
}

void Player::UpdateTransformVectors()
{
	btTransform transform{};
	mVehicleRigidBody.StoreWorldTransform(transform);

	mPosition = transform.getOrigin();
	auto quat = transform.getRotation();
	mQuaternion.setValue(quat.x(), quat.y(), quat.z(), quat.w());
}

void Player::UpdateVehicleComponent(float elapsed)
{
	auto& component = mVehicleRigidBody.GetComponent();

	if (component.VehicleSteering > 0)
	{
		component.VehicleSteering = std::max(
			component.VehicleSteering 
				- mConstantPtr->SteeringIncrement * elapsed, 
			0.0f);
	}
	else if (component.VehicleSteering < 0)
	{
		component.VehicleSteering = std::min(
			component.VehicleSteering 
				+ mConstantPtr->SteeringIncrement * elapsed,
			0.0f);
	}

	if (mKeyMap[VK_LEFT])
	{
		component.VehicleSteering = std::max(
			component.VehicleSteering 
				- mConstantPtr->SteeringIncrement * 2 * elapsed,
			-mConstantPtr->SteeringClamp);
	}
	if (mKeyMap[VK_RIGHT])
	{
		component.VehicleSteering = std::min(
			component.VehicleSteering 
				+ mConstantPtr->SteeringIncrement * 2 * elapsed,
			mConstantPtr->SteeringClamp);
	}
	if (mKeyMap[VK_UP])
	{
		component.EngineForce =
			(component.CurrentSpeed < component.MaxSpeed) ?
				mConstantPtr->MaxEngineForce : 0.0f;
	}
	if (mKeyMap[VK_DOWN])
	{
		component.EngineForce =
			(component.CurrentSpeed > -component.MaxSpeed) ?
				-mConstantPtr->MaxEngineForce : 0.0f;
	}
}

void Player::ToggleKeyValue(uint8_t key, bool pressed)
{
	if (mKeyMap.find(key) != mKeyMap.end())
		mKeyMap[key] = pressed;
}