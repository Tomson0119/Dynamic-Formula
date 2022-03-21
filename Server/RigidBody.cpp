#include "common.h"
#include "RigidBody.h"

RigidBody::RigidBody()
	: mRigidBody{ nullptr }, 
	  mPosition{ 0.0f, 0.0f, 0.0f },
	  mQuaternion{ 0.0f, 0.0f, 0.0f, 1.0f },
	  mLinearVelocity{ 0.0f, 0.0f, 0.0f },
	  mAngularVelocity{ 0.0f, 0.0f, 0.0f },
	  mFlag{ UPDATE_FLAG::NONE }
{
}

void RigidBody::CreateRigidBody(btScalar mass, btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	btVector3 inertia(0.0f, 0.0f, 0.0f);
	if (mass != 0.0f && shape)
	{
		shape->calculateLocalInertia(mass, inertia);
	}

	btTransform originTransform = btTransform::getIdentity();
	originTransform.setOrigin(mPosition);

	btDefaultMotionState* motionState = new btDefaultMotionState(originTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, inertia);
	mRigidBody = new btRigidBody(cInfo);
}

void RigidBody::Update(btDiscreteDynamicsWorld* physicsWorld)
{
	auto flag = GetUpdateFlag();
	switch (flag)
	{
	case RigidBody::UPDATE_FLAG::CREATION:
		AppendRigidBody(physicsWorld);
		break;

	case RigidBody::UPDATE_FLAG::UPDATE:
		UpdateRigidBody();
		return;

	case RigidBody::UPDATE_FLAG::DELETION:
		RemoveRigidBody(physicsWorld);
		break;

	case RigidBody::UPDATE_FLAG::NONE:
		return;
	}
	SetUpdateFlag(UPDATE_FLAG::UPDATE);
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

void RigidBody::SetTransform(const btVector3& position, const btQuaternion& rotation)
{
	btTransform newTransform{};
	newTransform.setOrigin(position);
	newTransform.setRotation(rotation);
	mRigidBody->setWorldTransform(newTransform);
}

void RigidBody::AppendRigidBody(btDiscreteDynamicsWorld* physicsWorld)
{
	if(mRigidBody)
		physicsWorld->addRigidBody(mRigidBody);
}

void RigidBody::UpdateRigidBody()
{
}

void RigidBody::RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld)
{
	if (mRigidBody)
	{
		auto motionState = mRigidBody->getMotionState();
		if (motionState)
		{
			delete motionState;
			motionState = nullptr;
		}

		physicsWorld->removeRigidBody(mRigidBody);
		delete mRigidBody;
		mRigidBody = nullptr;
	}
}

bool RigidBody::ChangeUpdateFlag(UPDATE_FLAG expected, UPDATE_FLAG desired)
{
	if (mFlag.compare_exchange_strong(expected, desired) == false)
	{
		// DELETION flag must be a priority.
		if (desired == UPDATE_FLAG::DELETION)
			mFlag = UPDATE_FLAG::DELETION;
		return false;
	}
	return true;
}


//
// VehicleRigidBody
//
VehicleRigidBody::VehicleRigidBody()
	: RigidBody()
{
}

void VehicleRigidBody::CreateRaycastVehicle(
	btDiscreteDynamicsWorld* physicsWorld,
	const btVector3& bodyExtents, 
	const BtCarShape::WheelInfo& wheelInfo)
{
	if (mRigidBody)
	{
		mRigidBody->setActivationState(DISABLE_DEACTIVATION);

		mVehicleRayCaster = std::make_unique<btDefaultVehicleRaycaster>(physicsWorld);
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

void VehicleRigidBody::AppendRigidBody(btDiscreteDynamicsWorld* physicsWorld)
{
	RigidBody::AppendRigidBody(physicsWorld);
	if(mVehicle) physicsWorld->addVehicle(mVehicle.get());
}

void VehicleRigidBody::RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld)
{
	RigidBody::RemoveRigidBody(physicsWorld);
	if (mVehicle) 
	{
		physicsWorld->removeVehicle(mVehicle.get());
		mVehicle.reset();
		mVehicleRayCaster.reset();
	}
}

void VehicleRigidBody::StoreWorldTransform(btTransform& transform)
{
	auto motionState = mVehicle->getRigidBody()->getMotionState();
	if(motionState) motionState->getWorldTransform(transform);
}

void VehicleRigidBody::UpdateRigidBody()
{
	mVehicle->getWheelInfo(0).m_frictionSlip = mComponent.FrictionSlip;
	mVehicle->getWheelInfo(1).m_frictionSlip = mComponent.FrictionSlip;
	mVehicle->getWheelInfo(2).m_frictionSlip = mComponent.FrictionSlip;
	mVehicle->getWheelInfo(3).m_frictionSlip = mComponent.FrictionSlip;

	mVehicle->applyEngineForce(mComponent.EngineForce, 0);
	mVehicle->applyEngineForce(mComponent.EngineForce, 1);
	mVehicle->setSteeringValue(mComponent.VehicleSteering, 0);
	mVehicle->setSteeringValue(mComponent.VehicleSteering, 1);

	mComponent.CurrentSpeed = mVehicle->getCurrentSpeedKmHour();
}


//
//	MapRigidBody
//
void MapRigidBody::CreateTerrainRigidBody(BtTerrainShape* shape)
{
	mStaticRigidBodies.emplace_back();

	RigidBody& terrainRigidBody = mStaticRigidBodies.back();
	terrainRigidBody.SetPosition(shape->GetOriginPosition());
	terrainRigidBody.CreateRigidBody(0.0f, shape->GetCollisionShape());
	terrainRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
}

void MapRigidBody::CreateStaticRigidBodies(std::string_view filename, btCollisionShape* shape)
{
	// TODO: Read position, scale, rotation values from file.
	//		 and create all rigidboies
}

void MapRigidBody::UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld)
{
	for (RigidBody& rigid : mStaticRigidBodies)
	{
		rigid.Update(physicsWorld);
	}
}
