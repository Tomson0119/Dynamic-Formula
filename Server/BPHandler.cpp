#include "common.h"
#include "BPHandler.h"
#include "GameObject.h"

BPHandler::BPHandler(float gravity)
{
	Init(gravity);
}

BPHandler::~BPHandler()
{
	if (mBtDynamicsWorld)
	{
		for (int i = mBtDynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
		{
			mBtDynamicsWorld->removeConstraint(mBtDynamicsWorld->getConstraint(i));
		}
	}
	Flush();
}

void BPHandler::Init(float gravity)
{
	mBtCollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	mBtDispatcher = std::make_unique<btCollisionDispatcher>(mBtCollisionConfiguration.get());
	mBtOverlappingPairCache = std::make_unique<btDbvtBroadphase>();
	mBtSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

	mBtDynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(mBtDispatcher.get(), mBtOverlappingPairCache.get(), mBtSolver.get(), mBtCollisionConfiguration.get());
	mBtDynamicsWorld->setGravity(btVector3(0, gravity, 0));
}

void BPHandler::StepSimulation(float elapsed)
{
	mBtDynamicsWorld->stepSimulation(elapsed, 2);
	CheckCollision();
}

void BPHandler::CheckCollision()
{
	int numManifolds = mBtDynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold =
			mBtDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

		if (contactManifold->getNumContacts() <= 0) continue;

		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();

		if (objA == nullptr || objB == nullptr) continue;

		GameObject* gameObjA = reinterpret_cast<GameObject*>(objA->getUserPointer());
		GameObject* gameObjB = reinterpret_cast<GameObject*>(objB->getUserPointer());

		if (gameObjA == nullptr || gameObjB == nullptr) continue;

		gameObjA->HandleCollisionWith(*objA, *objB, *gameObjB);
		gameObjB->HandleCollisionWith(*objB, *objA, *gameObjA);
	}
}

void BPHandler::Flush()
{
	if (mBtDynamicsWorld)
	{				
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
	}
}
