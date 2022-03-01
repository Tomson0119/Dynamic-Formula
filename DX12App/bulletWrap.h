#pragma once

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include <iostream>
#include <vector>

class BulletWrapper
{
public:
	BulletWrapper(const float& gravity);
	~BulletWrapper();

	btRigidBody* CreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape);

	std::shared_ptr<btDiscreteDynamicsWorld> GetDynamicsWorld() { return mBtDynamicsWorld; }

	void AddShape(btCollisionShape* shape) { mCollisionShapes.push_back(shape); }
	void StepSimulation(float elapsed);

	void SetTerrainRigidBodies(const std::vector<btRigidBody*>& TerrainRigidBodies);

private:
	std::shared_ptr<btDefaultCollisionConfiguration> mBtCollisionConfiguration;
	std::shared_ptr<btCollisionDispatcher> mBtDispatcher;
	std::shared_ptr<btBroadphaseInterface> mBtOverlappingPairCache;
	std::shared_ptr<btSequentialImpulseConstraintSolver> mBtSolver;
	std::shared_ptr<btDiscreteDynamicsWorld> mBtDynamicsWorld;

	btAlignedObjectArray<btRigidBody*> mTerrainRigidBodies;

	btAlignedObjectArray<btCollisionShape*> mCollisionShapes;
};