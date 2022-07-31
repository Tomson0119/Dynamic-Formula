#include "common.h"
#include "RigidBody.h"
#include "BPHandler.h"
#include "InGameServer.h"

RigidBody::RigidBody()
	: mRigidBody{ nullptr }, 
	  mPosition{ 0.0f, 0.0f, 0.0f },
	  mQuaternion{ 0.0f, 0.0f, 0.0f, 1.0f },
	  mLinearVelocity{ 0.0f, 0.0f, 0.0f },
	  mAngularVelocity{ 0.0f, 0.0f, 0.0f },
	  mMaskGroup{ 0 },
	  mMask{ 0 },
	  mFlag{ UPDATE_FLAG::NONE }
{
}

RigidBody::~RigidBody()
{
	Flush();
}

void RigidBody::Flush()
{
	if (mRigidBody)
	{
		auto motionState = mRigidBody->getMotionState();
		if (motionState) delete motionState;
		delete mRigidBody;
		mRigidBody = nullptr;
	}
}

void RigidBody::SetMaskBits(int maskGroup, int mask)
{
	mMaskGroup = maskGroup;
	mMask = mask;
}

void RigidBody::SetNoResponseCollision()
{
	if (mRigidBody)
	{
		mRigidBody->setCollisionFlags(
			mRigidBody->getCollisionFlags() |
			btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

void RigidBody::CreateRigidBody(btScalar mass, btCollisionShape& shape, GameObject* objPtr)
{
	btAssert(shape.getShapeType() != INVALID_SHAPE_PROXYTYPE);

	btVector3 inertia(0.0f, 0.0f, 0.0f);
	if (mass != 0.0f)
	{
		shape.calculateLocalInertia(mass, inertia);
	}

	btTransform originTransform = btTransform::getIdentity();
	originTransform.setOrigin(mPosition);
	originTransform.setRotation(mQuaternion);

	btDefaultMotionState* motionState = new btDefaultMotionState(originTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, &shape, inertia);
	mRigidBody = new btRigidBody(cInfo);
	mRigidBody->setUserPointer(objPtr);
}

void RigidBody::SetTransform(const btVector3& pos, const btQuaternion& quat)
{
	if (mRigidBody)
	{
		btTransform transform = btTransform::getIdentity();
		transform.setOrigin(pos);
		transform.setRotation(quat);
		mRigidBody->setWorldTransform(transform);
		mRigidBody->getMotionState()->setWorldTransform(transform);
	}
	mPosition = pos;
	mQuaternion = quat;
}

void RigidBody::SetLinearVelocity(const btVector3& vel)
{
	mRigidBody->setLinearVelocity(vel);
}

void RigidBody::SetAngularVelocity(const btVector3& vel)
{
	mRigidBody->setAngularVelocity(vel);
}

void RigidBody::Update(BPHandler& physics)
{
	auto flag = GetUpdateFlag();
	switch (flag)
	{
	case RigidBody::UPDATE_FLAG::CREATE:
		AppendRigidBody(physics);
		SetUpdateFlag(UPDATE_FLAG::UPDATE);
		break;

	case RigidBody::UPDATE_FLAG::UPDATE:
		UpdateRigidBody();		
		break;

	case RigidBody::UPDATE_FLAG::REMOVE:
		RemoveRigidBody(physics);
		SetUpdateFlag(UPDATE_FLAG::NONE);
		break;

	case RigidBody::UPDATE_FLAG::CHANGE_MASK:
		RemoveRigidBody(physics);
		AppendRigidBody(physics);
		SetUpdateFlag(RigidBody::UPDATE_FLAG::UPDATE);
		break;

	case RigidBody::UPDATE_FLAG::NONE:
		break;
	}	
}

void RigidBody::UpdateTransformVectors()
{
	if (mRigidBody)
	{
		btTransform transform{};
		mRigidBody->getMotionState()->getWorldTransform(transform);

		mPosition = transform.getOrigin();
		mQuaternion = transform.getRotation();
		mLinearVelocity = mRigidBody->getInterpolationLinearVelocity();
		mAngularVelocity = mRigidBody->getInterpolationAngularVelocity();
	}
}

void RigidBody::AppendRigidBody(BPHandler& physics)
{
	if (mRigidBody)
	{
		Helper::Assert((mMaskGroup > 0 && mMask > 0), "Mask bits are not assigned.");
		physics.AddRigidBody(mRigidBody, mMaskGroup, mMask);
	}
}

void RigidBody::UpdateRigidBody()
{
	UpdateTransformVectors();
}

void RigidBody::RemoveRigidBody(BPHandler& physics)
{
	if (mRigidBody)
	{
		physics.RemoveRigidBody(mRigidBody);
	}
}

bool RigidBody::ChangeUpdateFlag(UPDATE_FLAG expected, UPDATE_FLAG desired)
{
	if (mFlag.compare_exchange_strong(expected, desired) == false)
	{
		// DELETION flag must be a priority.
		if (desired == UPDATE_FLAG::REMOVE)
		{
			mFlag = UPDATE_FLAG::REMOVE;
			return true;
		}
		return false;
	}
	return true;
}

//
// MissileRigidBody
//
MissileRigidBody::MissileRigidBody()
	: RigidBody(), mVehiclePtr{ nullptr }, mActive{ false }, mLaunchFlag{ false }
{
}

void MissileRigidBody::AppendRigidBody(BPHandler& physics)
{
	RigidBody::AppendRigidBody(physics);
	if (mRigidBody) SetMissileComponents();
	
	mDestoryTime = Clock::now() + mConstantPtr->MissileLifetime;
	SetLaunchFlag(true);
}

void MissileRigidBody::UpdateRigidBody()
{
	if (IsActive())
	{				
		RigidBody::UpdateRigidBody();
	}
}

void MissileRigidBody::RemoveRigidBody(BPHandler& physics)
{
	if (mRigidBody) mRigidBody->setLinearVelocity({ 0.0f,0.0f,0.0f });
	RigidBody::RemoveRigidBody(physics);
}

void MissileRigidBody::SetGameConstantPtr(
	VehicleRigidBody* vehiclePtr,
	std::shared_ptr<GameConstant> constantPtr)
{
	mVehiclePtr = vehiclePtr;
	mConstantPtr = constantPtr;
}

bool MissileRigidBody::LifeTimeOver() const
{
	if (IsActive())
	{
		auto now = Clock::now();
		return (now >= mDestoryTime);
	}
	return false;
}

void MissileRigidBody::SetMissileComponents()
{
	btTransform newTransform = btTransform::getIdentity();

	btVector3 position = mVehiclePtr->GetPosition();
	position += mConstantPtr->MissileOffset;

	btVector3 forward = mVehiclePtr->GetForwardVector();
	if (forward.getY() < 0) forward.setY(0.0f);
	forward = forward.normalize();
	forward *= mConstantPtr->MissileForwardMag;

	newTransform.setOrigin(position + forward);
	newTransform.setRotation(mVehiclePtr->GetRotation());

	mRigidBody->getMotionState()->setWorldTransform(newTransform);
	mRigidBody->setWorldTransform(newTransform);
	mRigidBody->setGravity(mConstantPtr->MissileGravity);

	btVector3 linearVel = mVehiclePtr->GetLinearVelocity();	
	float speed = (float)linearVel.length() + mConstantPtr->MissileSpeed;
	btVector3 velocity = forward.normalize() * speed;
	SetLinearVelocity(velocity);
}


//
// CustomVehicleRaycaster
//
CustomVehicleRaycaster::CustomVehicleRaycaster(btDynamicsWorld* dynamicsWorld)
	: mDynamicsWorld{ dynamicsWorld }, mMaskGroup{ 0 }, mMask{ 0 }
{
}

void* CustomVehicleRaycaster::castRay(const btVector3& from, const btVector3& to, btVehicleRaycasterResult& result)
{
	btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);

	if (mMaskGroup > 0 && mMask)
	{
		rayCallback.m_collisionFilterGroup = mMaskGroup;
		rayCallback.m_collisionFilterMask = mMask;
	}
	
	mDynamicsWorld->rayTest(from, to, rayCallback);

	if (rayCallback.hasHit())
	{
		const btRigidBody* body = btRigidBody::upcast(rayCallback.m_collisionObject);
		if (body && body->hasContactResponse())
		{
			result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
			result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = rayCallback.m_closestHitFraction;
			return (void*)body;
		}
	}
	return 0;
}

void CustomVehicleRaycaster::SetMaskBits(int maskGroup, int mask)
{
	mMaskGroup = maskGroup;
	mMask = mask;
}


//
// VehicleRigidBody
//
VehicleRigidBody::VehicleRigidBody()
	: RigidBody()
{
}

void VehicleRigidBody::CreateRaycastVehicle(
	BPHandler& physics,
	const btVector3& bodyExtents, 
	const BtCarShape::WheelInfo& wheelInfo)
{
	if (mRigidBody)
	{
		mRigidBody->setActivationState(DISABLE_DEACTIVATION);

		mVehicleRayCaster = std::make_unique<CustomVehicleRaycaster>(physics.GetDynamicsWorld());
		mVehicle = std::make_unique<btRaycastVehicle>(mTuning, mRigidBody, mVehicleRayCaster.get());
		mVehicle->setCoordinateSystem(0, 1, 2);

		AddWheel(bodyExtents, wheelInfo);
	}
}

void VehicleRigidBody::AddWheel(const btVector3& bodyExtents, const BtCarShape::WheelInfo& wheelInfo)
{
	btScalar wheelRadius = wheelInfo.Extents.z();
	btScalar connectionHeight = wheelInfo.ConnectionHeight;

	btVector3 centerPoint = btVector3(bodyExtents.x(), connectionHeight, bodyExtents.z());
	for (int i = 0; i < 4; i++)
	{
		bool isFront = (i < 2);

		btVector3 connectionPointCS0 = centerPoint;
		if (isFront)
		{
			connectionPointCS0 += wheelInfo.FrontOffset;
		}
		else
		{
			connectionPointCS0 += wheelInfo.BackOffset;
			connectionPointCS0.setZ(connectionPointCS0.getZ() * -1.0f);
		}

		if (i % 2 == 1)
			connectionPointCS0.setX(connectionPointCS0.getX() * -1.0f);

		mVehicle->addWheel(
			connectionPointCS0,
			wheelInfo.DirectionCS0,
			wheelInfo.AxleCS,
			wheelInfo.SuspensionRestLength,
			wheelRadius, mTuning, isFront);

		btWheelInfo& wheel = mVehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = wheelInfo.SuspensionStiffness;
		wheel.m_wheelsDampingRelaxation = wheelInfo.SuspensionDamping;
		wheel.m_wheelsDampingCompression = wheelInfo.SuspensionCompression;
		wheel.m_frictionSlip = wheelInfo.Friction;
		wheel.m_rollInfluence = wheelInfo.RollInfluence;
	}
}

void VehicleRigidBody::AppendRigidBody(BPHandler& physics)
{
	RigidBody::AppendRigidBody(physics);
	if (mVehicle) 
	{
		mVehicleRayCaster->SetMaskBits(mMaskGroup, mMask);
		physics.AddVehicle(mVehicle.get());
	}
}

void VehicleRigidBody::RemoveRigidBody(BPHandler& physics)
{
	RigidBody::RemoveRigidBody(physics);
	if (mVehicle) 
	{
		physics.RemoveVehicle(mVehicle.get());
	}
}

void VehicleRigidBody::StoreWorldTransform(btTransform& transform)
{
	auto motionState = mVehicle->getRigidBody()->getMotionState();
	if(motionState) motionState->getWorldTransform(transform);
}

void VehicleRigidBody::UpdateRigidBody()
{
	RigidBody::UpdateRigidBody();

	mVehicle->getWheelInfo(0).m_frictionSlip = mComponent.FrontFrictionSlip;
	mVehicle->getWheelInfo(1).m_frictionSlip = mComponent.FrontFrictionSlip;
	mVehicle->getWheelInfo(2).m_frictionSlip = mComponent.BackFrictionSlip;
	mVehicle->getWheelInfo(3).m_frictionSlip = mComponent.BackFrictionSlip;

	mVehicle->applyEngineForce(mComponent.EngineForce, 0);
	mVehicle->applyEngineForce(mComponent.EngineForce, 1);
	mVehicle->setBrake(mComponent.BreakingForce, 0);
	mVehicle->setBrake(mComponent.BreakingForce, 1);
	mVehicle->setSteeringValue(mComponent.VehicleSteering, 0);
	mVehicle->setSteeringValue(mComponent.VehicleSteering, 1);

	mComponent.CurrentSpeed = mVehicle->getCurrentSpeedKmHour();
}