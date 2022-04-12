#pragma once

#include "GameObject.h"
#include "RigidBody.h"

class Map : public GameObject
{
public:
	Map();
	virtual ~Map() = default;

	virtual void UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld) override;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) override;

	void CreateTrackRigidBody(btCollisionShape& shape);
	void CreateCheckpoints(btCollisionShape& shape, const std::vector<CheckpointShape::CheckpointInfo>& infos);

private:
	RigidBody mTrack;
	std::deque<RigidBody> mCheckpoints;
};