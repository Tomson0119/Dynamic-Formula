#include "common.h"
#include "RigidBody.h"

RigidBody::RigidBody()
	: mRigidBody{ nullptr }, mFlag{ UPDATE_FLAG::NONE }
{
}

void RigidBody::CreateRigidBody(
	btScalar mass, 
	btCollisionShape* shape, 
	const btVector3& position, 
	const btVector3& offset)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	btVector3 inertia(0.0f, 0.0f, 0.0f);
	if (mass != 0.0f && shape)
	{
		shape->calculateLocalInertia(mass, inertia);
	}

	btTransform originTransform = btTransform::getIdentity();
	originTransform.setOrigin(position + offset);

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
		break;

	case RigidBody::UPDATE_FLAG::DELETION:
		RemoveRigidBody(physicsWorld);
		break;

	case UPDATE_FLAG::NONE:
		return;
	}
	SetUpdateFlag(UPDATE_FLAG::NONE);
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
		if (motionState) delete motionState;

		physicsWorld->removeRigidBody(mRigidBody);
		delete mRigidBody;
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
	if(mVehicle) physicsWorld->removeVehicle(mVehicle.get());
}

void VehicleRigidBody::UpdateRigidBody()
{
	// TODO: Update vehicle/wheel components and apply it.
}


//
//	MapRigidBody
//
void MapRigidBody::CreateTerrainRigidBody(BtTerrainShape* shape)
{
	mStaticRigidBodies.emplace_back();

	RigidBody& terrainRigidBody = mStaticRigidBodies.back();
	terrainRigidBody.CreateRigidBody(
		0.0f,
		shape->GetCollisionShape(),
		shape->GetOriginPosition());

	terrainRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
}

void MapRigidBody::CreateStaticRigidBodies(std::string_view filename, btCollisionShape* shape)
{
	// TODO: Read position, scale, rotation values from file.
	//		 and create all rigidboies
}

void MapRigidBody::UpdateAllRigidBody(btDiscreteDynamicsWorld* physicsWorld)
{
	for (RigidBody& rigid : mStaticRigidBodies)
	{
		rigid.Update(physicsWorld);
	}
}
