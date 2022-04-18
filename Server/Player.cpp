#include "common.h"
#include "Player.h"
#include "BtCompoundShape.h"
#include "RigidBody.h"
#include "Map.h"

Player::Player()
	: Empty{ true }, 
	  Color{ -1 }, 
	  Ready{ false }, 
	  ID{ -1 }, 
	  Name{ }, 
	  LoadDone{ false }, 
	  mCurrentCPIndex{ -1 },
	  mLapCount{ 0 },
	  mDriftGauge{ 0.0f },
	  mPoint{ 0 },
	  mItemCount{ 0 }, 
	  mInvincible{ false },
	  mBoosterToggle{ false } 
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
		mVehicleRigidBody.CreateRigidBody(mass,	shape->GetCompoundShape(), this);

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
		mMissileRigidBody.CreateRigidBody(mass, shape->GetCollisionShape(), this);
		mMissileRigidBody.SetNoResponseCollision();
	}
}

void Player::Update(float elapsed, btDiscreteDynamicsWorld* physicsWorld)
{
	UpdateVehicleComponent(elapsed);
	// Update invincible mode duration.

	mVehicleRigidBody.Update(physicsWorld);
	mMissileRigidBody.Update(physicsWorld);
}

void Player::SetDeletionFlag()
{
	mMissileRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION);
	mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION);
}

void Player::Reset(btDiscreteDynamicsWorld* physicsWorld)
{
	LoadDone = false;
	mMissileRigidBody.RemoveRigidBody(physicsWorld);
	mVehicleRigidBody.RemoveRigidBody(physicsWorld);
}

void Player::HandleCollisionWith(const btCollisionObject& objA, const btCollisionObject& objB, GameObject& otherObj)
{
	auto myTag = GetTag(objA);
	auto otherTag = otherObj.GetTag(objB);
	switch (myTag)
	{
	case OBJ_TAG::VEHICLE:
	{
		if (otherTag == OBJ_TAG::CHECKPOINT)
		{
			int cpIdx = dynamic_cast<Map&>(otherObj).GetCheckpointIndex(objB);
			HandleCheckpointCollision(cpIdx);
		}
		else if (otherTag == OBJ_TAG::MISSILE)
		{
			if (mInvincible == false)
			{
				mInvincible = true;
				std::cout << "Hit by missile!\n";
				// TODO: waits for like 1 second and send spawn packet.
				// and waits another 1 second and release invincible mode.
			}
		}
		break;
	}
	case OBJ_TAG::MISSILE:
	{
		if (otherTag == OBJ_TAG::VEHICLE || otherTag == OBJ_TAG::TRACK)
		{
			mMissileRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION);
		}
		else if (otherTag == OBJ_TAG::VEHICLE)
		{
			bool isInvincible = dynamic_cast<Player&>(otherObj).mInvincible;
			if (isInvincible == false)
			{
				mPoint += mConstantPtr->MissileHitPoint;
			}
		}
		break;
	}
	default:
		std::cout << "Wrong tag" << std::endl;
		break;
	}
}

GameObject::OBJ_TAG Player::GetTag(const btCollisionObject& obj) const
{
	if (&obj == mVehicleRigidBody.GetRigidBody())
	{
		return OBJ_TAG::VEHICLE;
	}
	else if (&obj == mMissileRigidBody.GetRigidBody())
	{
		return OBJ_TAG::MISSILE;
	}
}

void Player::HandleCheckpointCollision(int cpIndex)
{
	int nextIdx = (mCurrentCPIndex + 1) % mCPPassed.size();
	if (nextIdx == cpIndex && mCPPassed[nextIdx] == false)
	{
		std::cout << "Hit checkpoint: " << nextIdx << std::endl;
		if (mCurrentCPIndex >= 0)
		{
			mCPPassed[mCurrentCPIndex] = false;
			if (nextIdx == 0)
			{
				mLapCount += 1;
				mPoint += mConstantPtr->LapFinishPoint;
				std::cout << "Gets point!" << std::endl;
			}
		}
		mCurrentCPIndex = nextIdx;
		mCPPassed[nextIdx] = true;
	}
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
		comp.FrontFrictionSlip = mConstantPtr->FrontWheelDriftFriction;
		comp.BackFrictionSlip = mConstantPtr->RearWheelDriftFriction;

		auto linearVelocity = mVehicleRigidBody.GetLinearVelocity();
		auto forward = mVehicleRigidBody.GetForwardVector();

		if (linearVelocity.isZero() || forward.isZero()) return;

		linearVelocity.setY(0);
		forward.setY(0);

		auto linearVelNorm = linearVelocity.normalized();
		auto forwardNorm = forward.normalized();

		float angle = acos(linearVelNorm.dot(forwardNorm) / linearVelNorm.length() * forwardNorm.length());

		float DriftLimit = 30.0f / 180.0f;
		float AngleLimit = 50.0f / 180.0f;

		if (angle > DriftLimit && mDriftGauge < 1.0f)
		{
			mDriftGauge += elapsed * 0.5f;
			if (mDriftGauge > 1.0f)
			{
				mDriftGauge = 0.0f;
				if (mItemCount < 2)	mItemCount += 1;	
			}
		}

		if (angle > AngleLimit)
		{
			mVehicleRigidBody.SetAngularVelocity(btVector3(0.f, 0.f, 0.f));
		}
	}
	else
	{
		comp.FrontFrictionSlip = mConstantPtr->WheelDefaultFriction;
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

	bool handled = false;
	comp.EngineForce = 0.0f;

	if (comp.BoosterTimeLeft > 0.0f && comp.MaxSpeed > comp.CurrentSpeed)
	{
		comp.EngineForce = mConstantPtr->BoosterEngineForce;
		return;
	}

	if (mKeyMap[VK_UP])
	{
		handled = true;
		if (comp.MaxSpeed > comp.CurrentSpeed)
		{
			comp.EngineForce = mConstantPtr->MaxEngineForce;
		}
		else
		{
			comp.EngineForce = 0.0f;
		}
	}
	if (mKeyMap[VK_DOWN])
	{
		handled = true;
		if (comp.CurrentSpeed > -comp.MaxSpeed)
		{
			comp.EngineForce = -mConstantPtr->MaxEngineForce;
		}
		else
		{
			comp.EngineForce = 0.0f;
		}
	}

	if (!handled)
		comp.BreakingForce = mConstantPtr->DefaultBreakingForce;
}


void Player::ToggleKeyValue(uint8_t key, bool pressed)
{
	if ((key == 'Z' || key == 'X'))
	{
		if (pressed && IsItemAvailable())
		{
			//if (UseItem(key)) mItemCount -= 1;
			UseItem(key);
		}
	}
	else
	{
		mKeyMap[key] = pressed;
	}
}

bool Player::IsItemAvailable()
{
	//return (mItemCount > 0);
	return true;
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