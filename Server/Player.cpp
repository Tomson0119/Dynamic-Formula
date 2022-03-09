#include "common.h"
#include "Player.h"
#include "BtShape.h"
#include "RigidBody.h"

Player::Player()
	: mPosition{ 0.0f, 0.0f, 0.0f },
	  Empty{ true }, Color{ -1 }, Ready{ false }, 
	  ID{ -1 }, Name{ }, LoadDone{ false },
	  mPrevVelocity{ }, mCurrVelocity{ }, mAcceleration{ }
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
			shape->GetCompoundShape(), mPosition);

		mVehicleRigidBody.CreateRaycastVehicle(
			physicsWorld, shape->GetExtents(),
			shape->GetWheelInfo());

		ClearVehicleComponent();

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

void Player::ClearVehicleComponent()
{
	auto& comp = mVehicleRigidBody.GetComponent();
	comp.BoosterLeft	 = 0.0f;
	comp.CurrentSpeed	 = 0.0f;
	comp.EngineForce	 = 0.0f;
	comp.VehicleSteering = 0.0f;
	comp.FrictionSlip	 = mConstantPtr->WheelDefaultFriction;
	comp.MaxSpeed		 = mConstantPtr->DefaultMaxSpeed;

	for (auto& [key, val] : mKeyMap) val = false;
}

void Player::UpdateVehicleComponent(float elapsed)
{
	CalculateAcceleration(elapsed);
	UpdateSteering(elapsed);
	UpdateEngineForce();
}

void Player::UpdateSteering(float elapsed)
{
	auto& component = mVehicleRigidBody.GetComponent();
	if (component.VehicleSteering > 0)
	{
		component.VehicleSteering = std::max(
			component.VehicleSteering - mConstantPtr->SteeringIncrement * elapsed,
			0.0f);
	}
	else if (component.VehicleSteering < 0)
	{
		component.VehicleSteering = std::min(
			component.VehicleSteering + mConstantPtr->SteeringIncrement * elapsed,
			0.0f);
	}
	if (mKeyMap[VK_LEFT])
	{
		component.VehicleSteering = std::max(
			component.VehicleSteering - mConstantPtr->SteeringIncrement * 2 * elapsed,
			-mConstantPtr->SteeringClamp);
	}
	if (mKeyMap[VK_RIGHT])
	{
		component.VehicleSteering = std::min(
			component.VehicleSteering + mConstantPtr->SteeringIncrement * 2 * elapsed,
			mConstantPtr->SteeringClamp);
	}
}

void Player::UpdateEngineForce()
{
	auto& component = mVehicleRigidBody.GetComponent();
	component.EngineForce = 0.0f;
	if (mKeyMap[VK_UP])
	{
		if (component.CurrentSpeed < 0.0f)
			component.EngineForce = mConstantPtr->MaxEngineForce * 1.5f;
		else if (component.MaxSpeed > component.CurrentSpeed)
			component.EngineForce = mConstantPtr->MaxEngineForce;
		else
			component.EngineForce = 0.0f;
	}
	if (mKeyMap[VK_DOWN])
	{
		if (component.CurrentSpeed > 0.0f)
			component.EngineForce = -mConstantPtr->MaxEngineForce * 1.5f;
		else if (component.CurrentSpeed > -component.MaxSpeed)
			component.EngineForce = -mConstantPtr->MaxEngineForce;
		else
			component.EngineForce = 0.0f;
	}
}

void Player::CalculateAcceleration(float elapsed)
{
	const float mps = 0.277778f;
	mCurrVelocity = mVehicleRigidBody.GetVehicle()->getForwardVector()
		* mVehicleRigidBody.GetComponent().CurrentSpeed * mps;
	
	mAcceleration = (mCurrVelocity - mPrevVelocity) / elapsed;
	mPrevVelocity = mCurrVelocity;
}

void Player::ToggleKeyValue(uint8_t key, bool pressed)
{
	if (mKeyMap.find(key) != mKeyMap.end())
		mKeyMap[key] = pressed;
}