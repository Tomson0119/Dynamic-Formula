#include "common.h"
#include "Player.h"
#include "BtShape.h"
#include "RigidBody.h"

Player::Player()
	: Empty{ true }, Color{ -1 }, Ready{ false }, 
	  ID{ -1 }, Name{ }, LoadDone{ false }
{
	mKeyMap[VK_UP]	   = false;
	mKeyMap[VK_DOWN]   = false;
	mKeyMap[VK_LEFT]   = false;
	mKeyMap[VK_RIGHT]  = false;
	mKeyMap[VK_LSHIFT] = false;
}

void Player::SetPosition(const btVector3& pos)
{
	mVehicleRigidBody.SetPosition(pos);
	mMissileRigidBody.SetPosition(pos);
}

void Player::SetRotation(const btQuaternion& quat)
{
	mVehicleRigidBody.SetRotation(quat);
	mMissileRigidBody.SetRotation(quat);
}

void Player::SetBulletConstant(std::shared_ptr<InGameServer::BulletConstant> constantPtr)
{
	mConstantPtr = constantPtr;
	mMissileRigidBody.SetVehicleAndConstantPtr(&mVehicleRigidBody, constantPtr);
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
			shape->GetCompoundShape());

		mVehicleRigidBody.CreateRaycastVehicle(
			physicsWorld, shape->GetExtents(),
			shape->GetWheelInfo());

		ClearVehicleComponent();

		mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
	}
}

void Player::CreateMissileRigidBody(btScalar mass, BtBoxShape* shape)
{
	if (shape)
	{
		mMissileRigidBody.CreateRigidBody(mass, shape->GetCollisionShape());
	}
}

void Player::UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld)
{
	UpdateVehicleComponent(elapsed);
	mVehicleRigidBody.Update(physicsWorld);
	mMissileRigidBody.Update(physicsWorld);
}

void Player::SetDeletionFlag()
{
	mMissileRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION);
	mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION);
}

void Player::ResetPlayer(btDiscreteDynamicsWorld* physicsWorld)
{
	LoadDone = false;
	mMissileRigidBody.RemoveRigidBody(physicsWorld);
	mVehicleRigidBody.RemoveRigidBody(physicsWorld);
}

void Player::UpdateWorldTransform()
{
	mVehicleRigidBody.UpdateTransformVectors();
	mMissileRigidBody.UpdateTransformVectors();
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

void Player::UpdateDiftGauge(float elapsed)
{
}

void Player::UpdateVehicleComponent(float elapsed)
{
	UpdateDiftGauge(elapsed);
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

bool Player::CheckDriftGauge()
{
	return true;
}

void Player::ToggleKeyValue(uint8_t key, bool pressed)
{
	if ((key == 'Z' || key == 'X') && pressed && CheckDriftGauge())
	{
		if (key == 'X')
		{
			mMissileRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
		}
	}
	else
	{
		mKeyMap[key] = pressed;
	}
}

bool Player::CheckMissileExist() const
{
	return (mMissileRigidBody.GetUpdateFlag() == RigidBody::UPDATE_FLAG::UPDATE);
}
