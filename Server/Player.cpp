#include "common.h"
#include "Player.h"
#include "BtCompoundShape.h"
#include "RigidBody.h"

Player::Player()
	: Empty{ true }, Color{ -1 }, Ready{ false }, 
	  ID{ -1 }, Name{ }, LoadDone{ false }, 
	  mBoosterToggle{ false }, mDriftGauge{ 0.0f },
	  mItemCount{ 0 }
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
	comp.BoosterTimeLeft   = 0.0f;
	comp.CurrentSpeed	   = 0.0f;
	comp.EngineForce	   = 0.0f;
	comp.VehicleSteering   = 0.0f;
	comp.FrontFrictionSlip = mConstantPtr->WheelDefaultFriction;
	comp.BackFrictionSlip  = mConstantPtr->WheelDefaultFriction;
	comp.MaxSpeed		   = mConstantPtr->DefaultMaxSpeed;
	comp.BreakingForce	   = 0.0f;

	for (auto& [key, val] : mKeyMap) val = false;
}

void Player::UpdateVehicleComponent(float elapsed)
{
	UpdateDriftGauge(elapsed);
	UpdateBooster(elapsed);
	UpdateSteering(elapsed);
	UpdateEngineForce();
}

void Player::UpdateDriftGauge(float elapsed)
{
	float currentSpeed = mVehicleRigidBody.GetCurrentSpeed();
	if (currentSpeed < mConstantPtr->MinSpeedForDrift) return;

	auto& comp = mVehicleRigidBody.GetComponent();

	if (mKeyMap[VK_LSHIFT])
	{
		comp.BackFrictionSlip = mConstantPtr->WheelDriftFriction;

		auto linearVelocity = mVehicleRigidBody.GetLinearVelocity();
		auto forward = mVehicleRigidBody.GetForwardVector();

		if (linearVelocity.isZero() || forward.isZero()) return;

		linearVelocity.setY(0);
		forward.setY(0);

		auto linearVelNorm = linearVelocity.normalized();
		auto forwardNorm = forward.normalized();

		float angle = acos(linearVelNorm.dot(forwardNorm));
		
		if (angle > Math::PI / 18.0f && mDriftGauge < 1.0f)
		{
			mDriftGauge += elapsed * 0.5f;
			if (mDriftGauge > 1.0f)
			{
				mDriftGauge = 0.0f;
				if (mItemCount < 2)	mItemCount += 1;	
			}
		}
	}
	else
	{
		comp.BackFrictionSlip = mConstantPtr->WheelDefaultFriction;
	}
}

void Player::UpdateBooster(float elapsed)
{
	auto& comp = mVehicleRigidBody.GetComponent();
	if (mBoosterToggle && comp.BoosterTimeLeft == 0.0f)
	{
		comp.BoosterTimeLeft = mConstantPtr->MaxBoosterTime;
		mBoosterToggle = false;
	}

	if (comp.BoosterTimeLeft > 0.0f)
	{
		comp.MaxSpeed = mConstantPtr->BoostedMaxSpeed;
		comp.BoosterTimeLeft -= elapsed;
	}
	else if (comp.BoosterTimeLeft < 0.0f)
	{
		comp.MaxSpeed = mConstantPtr->DefaultMaxSpeed;
		comp.BoosterTimeLeft = 0.0f;
	}
}

void Player::UpdateSteering(float elapsed)
{
	auto& comp = mVehicleRigidBody.GetComponent();
	float scale = (comp.BoosterTimeLeft > 0.0f) ? 0.5f : 2.0f;

	if (comp.VehicleSteering > 0)
	{
		comp.VehicleSteering = std::max(
			comp.VehicleSteering - mConstantPtr->SteeringIncrement * elapsed,
			0.0f);
	}
	else if (comp.VehicleSteering < 0)
	{
		comp.VehicleSteering = std::min(
			comp.VehicleSteering + mConstantPtr->SteeringIncrement * elapsed,
			0.0f);
	}
	if (mKeyMap[VK_LEFT])
	{		
		comp.VehicleSteering = std::max(
			comp.VehicleSteering - mConstantPtr->SteeringIncrement * scale * elapsed,
			-mConstantPtr->SteeringClamp);
	}
	if (mKeyMap[VK_RIGHT])
	{
		comp.VehicleSteering = std::min(
			comp.VehicleSteering + mConstantPtr->SteeringIncrement * scale * elapsed,
			mConstantPtr->SteeringClamp);
	}
}

void Player::UpdateEngineForce()
{
	auto& comp = mVehicleRigidBody.GetComponent();

	if (comp.BoosterTimeLeft > 0.0f && comp.MaxSpeed > comp.CurrentSpeed)
	{
		comp.EngineForce = mConstantPtr->BoosterEngineForce;
		return;
	}

	comp.EngineForce = 0.0f;
	comp.BreakingForce = mConstantPtr->DefaultBreakingForce;

	if (mKeyMap[VK_UP])
	{
		if (comp.CurrentSpeed < 0.0f)
		{
			comp.BreakingForce = mConstantPtr->MaxBreakingForce;
		}
		else if (comp.MaxSpeed > comp.CurrentSpeed)
		{
			comp.EngineForce = mConstantPtr->MaxEngineForce;
		}
		else
		{
			comp.BreakingForce = mConstantPtr->SubBreakingForce;
			comp.EngineForce = 0.0f;
		}
	}
	if (mKeyMap[VK_DOWN])
	{
		if (comp.CurrentSpeed > 0.0f)
		{
			comp.BreakingForce = mConstantPtr->MaxBreakingForce;
		}
		else if (comp.CurrentSpeed > -comp.MaxSpeed)
		{
			comp.EngineForce = -mConstantPtr->MaxBackwardEngineForce;
		}
		else
		{
			comp.BreakingForce = mConstantPtr->SubBreakingForce;
			comp.EngineForce = 0.0f;
		}
	}
}

void Player::ToggleKeyValue(uint8_t key, bool pressed)
{
	if ((key == 'Z' || key == 'X'))
	{
		if (pressed && IsItemAvailable())
		{
			if (UseItem(key)) mItemCount -= 1;
		}
	}
	else
	{
		mKeyMap[key] = pressed;
	}
}

bool Player::IsItemAvailable()
{
	return (mItemCount > 0);
}

bool Player::UseItem(uint8_t key)
{
	switch (key)
	{
	case 'Z':
	{
		bool b = false;
		return mBoosterToggle.compare_exchange_strong(b, true);
	}
	case 'X':
	{
		return mMissileRigidBody.ChangeUpdateFlag(
			RigidBody::UPDATE_FLAG::NONE,
			RigidBody::UPDATE_FLAG::CREATION);
	}}
	return false;
}

bool Player::CheckMissileExist() const
{
	return (mMissileRigidBody.GetUpdateFlag() == RigidBody::UPDATE_FLAG::UPDATE);
}
