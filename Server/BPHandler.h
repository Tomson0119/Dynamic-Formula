#pragma once

#include "MeshData.h"

class BPHandler
{
public:
	BPHandler() = default;
	BPHandler(float gravity);
	~BPHandler();

	void Init(float gravity);
	void LoadCollisionMeshes();

private:
	// 나중에 저장하지 않도록 테스트
	std::unique_ptr<btDefaultCollisionConfiguration> mBtCollisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> mBtDispatcher;
	std::unique_ptr<btBroadphaseInterface> mBtOverlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> mBtSolver;

	std::unique_ptr<btDiscreteDynamicsWorld> mBtDynamicsWorld;

	//std::vector<std::unique_ptr<MeshData>> mMeshes;
};