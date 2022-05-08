#include "common.h"
#include "Player.h"
#include "BtCompoundShape.h"
#include "RigidBody.h"
#include "Map.h"
#include "ObjectMask.h"
#include "InGameServer.h"


Player::Player()
	: Empty{ true }, 
	  Color{ -1 }, 
	  Ready{ false }, 
	  ID{ -1 }, 
	  Name{ }, 
	  LoadDone{ false }, 
	  mCurrentCPIndex{ -1 },
	  mLapCount{ 0 },
	  mHitCount{ 0 },
	  mPoint{ 0 },
	  mReverseDriveCount{ 0 },
	  mDriftGauge{ 0.0f },
	  mInvincibleDuration{ 0.0f },
	  mInvincible{ false },
	  mManualRespawn{ false },
	  mActive{ true },
	  mItemCount{ 0 },
	  mItemIncreased{ false },
	  mBoosterToggle{ false }
{
	mKeyMap[VK_UP]	   = false;
	mKeyMap[VK_DOWN]   = false;
	mKeyMap[VK_LEFT]   = false;
	mKeyMap[VK_RIGHT]  = false;
	mKeyMap[VK_LSHIFT] = false;
}

void Player::SetTransform(const btVector3& pos, const btQuaternion& quat)
{
	mVehicleRigidBody.SetTransform(pos, quat);
	mMissileRigidBody.SetTransform(pos, quat);
}

void Player::SetGameConstant(std::shared_ptr<GameConstant> constantPtr)
{
	mConstantPtr = constantPtr;
	mMissileRigidBody.SetGameConstantPtr(&mVehicleRigidBody, constantPtr);
}

void Player::CreateVehicleRigidBody(btScalar mass, BPHandler& physics, BtCarShape& shape)
{	
	mVehicleRigidBody.SetMaskBits(OBJ_MASK_GROUP::VEHICLE, OBJ_MASK::VEHICLE);
	mVehicleRigidBody.CreateRigidBody(mass,	shape.GetCompoundShape(), this);

	mVehicleRigidBody.CreateRaycastVehicle(
		physics, shape.GetExtents(),
		shape.GetWheelInfo());

	ClearVehicleComponent();
	ClearAllGameInfo();

	mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATE);
}

void Player::CreateMissileRigidBody(btScalar mass, BtCompoundShape& shape)
{
	mMissileRigidBody.SetMaskBits(OBJ_MASK_GROUP::MISSILE, OBJ_MASK::MISSILE);
	mMissileRigidBody.CreateRigidBody(mass, shape.GetCompoundShape(), this);
	mMissileRigidBody.SetNoResponseCollision();	
}

void Player::StopVehicle()
{
	mVehicleRigidBody.SetLinearVelocity(btVector3{ 0.0f,0.0f,0.0f });
}

void Player::ChangeVehicleMaskGroup(int maskGroup, BPHandler& physics)
{
	mVehicleRigidBody.ChangeUpdateFlag(RigidBody::UPDATE_FLAG::UPDATE, RigidBody::UPDATE_FLAG::CHANGE_MASK);
	mVehicleRigidBody.SetMaskBits(maskGroup, OBJ_MASK::VEHICLE);
}

void Player::Update(float elapsed, BPHandler& physics)
{
	UpdateVehicleComponent(elapsed);
	mVehicleRigidBody.Update(physics);
	mMissileRigidBody.Update(physics);
}

// NOTE: Not thread-safe.
void Player::SetRemoveFlag()
{
	mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::REMOVE);
	DisableMissile();
}

void Player::DisableMissile()
{
	mMissileRigidBody.Deactivate();
	mMissileRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::REMOVE);
}

void Player::SetInvincible(float duration)
{
	mInvincible = true;
	mActive = false;
	mInvincibleDuration = duration;
}

void Player::Reset(BPHandler& physics)
{
	LoadDone = false;
	mVehicleRigidBody.RemoveRigidBody(physics);
	mMissileRigidBody.RemoveRigidBody(physics);
	mVehicleRigidBody.Flush();
	mMissileRigidBody.Flush();
}

int Player::GetMask(const btCollisionObject& obj) const
{
	if (&obj == mVehicleRigidBody.GetRigidBody())
	{
		return OBJ_MASK::VEHICLE;
	}
	else if (&obj == mMissileRigidBody.GetRigidBody()
		&& mMissileRigidBody.IsActive())
	{
		return OBJ_MASK::MISSILE;
	}
	return OBJ_MASK::NONE;
}

bool Player::IsNextCheckpoint(int cpIndex)
{
	int nextIdx = (mCurrentCPIndex + 1) % mCPPassed.size();
	return (nextIdx == cpIndex && mCPPassed[nextIdx] == false);
}

void Player::MarkNextCheckpoint(int cpIndex)
{
	if (mCurrentCPIndex >= 0)
	{
		mCPPassed[mCurrentCPIndex] = false;
	}
	mCurrentCPIndex = cpIndex;
	mCPPassed[cpIndex] = true;
	ResetReverseCount();
}

int Player::GetReverseDriveCount(int cpIndex)
{
	int prevFirstIndex = std::max(0, mCurrentCPIndex) - 1;
	int prevSecondIndex = std::max(0, mCurrentCPIndex) - 2;

	if (prevFirstIndex < 0) prevFirstIndex = (int)mCPPassed.size() + prevFirstIndex;
	if (prevSecondIndex < 0) prevSecondIndex = (int)mCPPassed.size() + prevSecondIndex;

	if (cpIndex <= prevSecondIndex && mReverseDriveCount == 1)
	{
		mReverseDriveCount += 1;
		return mReverseDriveCount;
	}
	if (cpIndex == prevFirstIndex && mReverseDriveCount == 0)
	{
		mReverseDriveCount += 1;
		return mReverseDriveCount;
	}
	return 0;
}

bool Player::NeedUpdate()
{
	if (mVehicleRigidBody.GetUpdateFlag() == RigidBody::UPDATE_FLAG::REMOVE
		|| mMissileRigidBody.GetUpdateFlag() == RigidBody::UPDATE_FLAG::REMOVE)
	{
		return true;
	}
	return (Empty == false);
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

void Player::ClearAllGameInfo()
{
	for (int i = 0; i < mCPPassed.size(); i++)
	{
		mCPPassed[i] = false;
	}
	mCurrentCPIndex		= -1;
	mLapCount			= 0;
	mHitCount			= 0;
	mPoint				= 0;
	mReverseDriveCount	= 0;
	mItemCount			= 0;
	mDriftGauge			= 0.0f;
	mInvincibleDuration = 0.0f;
	mInvincible			= false;
	mItemIncreased		= false;
	mManualRespawn		= false;
	mActive				= true;
	mBoosterToggle		= false;
}

void Player::UpdateVehicleComponent(float elapsed)
{
	UpdateDriftGauge(elapsed);
	UpdateBooster(elapsed);
	UpdateSteering(elapsed);
	UpdateEngineForce();
}

void Player::UpdateInvincibleDuration(float elapsed)
{
	mInvincibleDuration -= elapsed;
}

void Player::UpdateDriftGauge(float elapsed)
{
	float currentSpeed = mVehicleRigidBody.GetCurrentSpeed();
	auto& comp = mVehicleRigidBody.GetComponent();

	if (mKeyMap[VK_LSHIFT])
	{
		if (currentSpeed < mConstantPtr->MinSpeedForDrift) return;

		comp.FrontFrictionSlip = mConstantPtr->FrontWheelDriftFriction;
		comp.BackFrictionSlip = mConstantPtr->RearWheelDriftFriction;

		auto linearVelocity = mVehicleRigidBody.GetLinearVelocity();
		auto forward = mVehicleRigidBody.GetForwardVector();

		if (linearVelocity.isZero() || forward.isZero()) return;

		linearVelocity.setY(0);
		forward.setY(0);

		auto linearVelNorm = linearVelocity.normalized();
		auto forwardNorm = forward.normalized();

		float angle = acos(linearVelNorm.dot(forwardNorm));

		float DriftLimit = 30.0f / 180.0f * (float)Math::PI;
		float AngleLimit = 50.0f / 180.0f * (float)Math::PI;

		if (angle > DriftLimit && mDriftGauge < 1.0f)
		{
			mDriftGauge += elapsed * 0.5f;
			if (mDriftGauge > 1.0f)
			{
				mDriftGauge = 0.0f;
				if (mItemCount < 2)
				{
					mItemCount += 1;
					mItemIncreased = true;
					std::cout << "Item increased.\n";
				}
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
	float scale = (1 - (comp.CurrentSpeed / (mConstantPtr->BoostedMaxSpeed + 50.0f))) * 2.0f;

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
	if (mActive == false) return;

	if ((key == 'Z' || key == 'X'))
	{
		if (pressed && IsItemAvailable())
		{
			// for test
			if (UseItem(key) && mItemCount > 0) mItemCount -= 1;
		}
	}
	else if (key == 'P' && pressed)
	{
		bool expected = false;
		mManualRespawn.compare_exchange_strong(expected, true);
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
		if (mMissileRigidBody.ChangeUpdateFlag(
			RigidBody::UPDATE_FLAG::NONE,
			RigidBody::UPDATE_FLAG::CREATE))
		{
			mMissileRigidBody.Activate();
			return true;
		}
	}}
	return false;
}

bool Player::CheckMissileExist() const
{
	return (mMissileRigidBody.IsActive());
}