#pragma once

class BPHandler
{
public:
	BPHandler() = default;
	BPHandler(float gravity);
	~BPHandler();

	void Init(float gravity);
	void AddRigidBody(btRigidBody* rigidbody) { mBtDynamicsWorld->addRigidBody(rigidbody); }
	void StepSimulation(float elapsed);

	void Flush();

public:
	btDiscreteDynamicsWorld* GetDynamicsWorld() const { return mBtDynamicsWorld.get(); }
	int GetNumManifolds() const { return mBtDynamicsWorld->getDispatcher()->getNumManifolds(); }
	btPersistentManifold* GetPersistentManifold(int idx) const { return mBtDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(idx); }

private:
	std::unique_ptr<btDefaultCollisionConfiguration> mBtCollisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> mBtDispatcher;
	std::unique_ptr<btBroadphaseInterface> mBtOverlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> mBtSolver;

	std::unique_ptr<btDiscreteDynamicsWorld> mBtDynamicsWorld;
};