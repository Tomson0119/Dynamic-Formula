#include "bulletWrap.h"

BulletWrapper::BulletWrapper(float gravity)
{
	mBtCollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

	mBtDispatcher = std::make_unique<btCollisionDispatcher>(mBtCollisionConfiguration.get());

	mBtOverlappingPairCache = std::make_unique<btDbvtBroadphase>();

	mBtSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

	mBtDynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(mBtDispatcher.get(), mBtOverlappingPairCache.get(), mBtSolver.get(), mBtCollisionConfiguration.get());
	mBtDynamicsWorld->setGravity(btVector3(0, gravity, 0));
}

BulletWrapper::~BulletWrapper()
{
	auto objArray = mBtDynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < mTerrainRigidBodies.size(); ++i)
	{
		if(objArray.findLinearSearch2(mTerrainRigidBodies[i]) == -1)
			mBtDynamicsWorld->addRigidBody(mTerrainRigidBodies[i]);
	}

	for (int i = mBtDynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
	{
		mBtDynamicsWorld->removeConstraint(mBtDynamicsWorld->getConstraint(i));
	}

	for (int i = mBtDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = mBtDynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		mBtDynamicsWorld->removeCollisionObject(obj);
		if (obj) delete obj;
	}

	for (int j = 0; j < mCollisionShapes.size(); j++)
	{
		btCollisionShape* shape = mCollisionShapes[j];
		mCollisionShapes[j] = 0;
		delete shape;
	}

	mCollisionShapes.clear();
}

btRigidBody* BulletWrapper::CreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, -1, 0);

	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);

	mBtDynamicsWorld->addRigidBody(body);
	AddShape(shape);

	return body;
}

void BulletWrapper::StepSimulation(float elapsed)
{
	/*btScalar timeStep = mPhysicsTimer.getTimeSeconds();
	mPhysicsTimer.reset();*/
	mBtDynamicsWorld->stepSimulation(elapsed, 2);
}

void BulletWrapper::SetTerrainRigidBodies(const std::vector<btRigidBody*>& TerrainRigidBodies)
{
	for (int i = 0; i < TerrainRigidBodies.size(); ++i)
	{
		mTerrainRigidBodies.push_back(TerrainRigidBodies[i]);
	}
}
