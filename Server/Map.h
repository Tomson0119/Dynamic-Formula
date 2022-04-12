#pragma once

#include "CollisionObject.h"
#include "RigidBody.h"

class Map : public CollisionObject
{
public:
	Map();
	virtual ~Map() = default;

	virtual void UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld) override;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) override;

	virtual void HandleCollisionWith(const OBJ_TAG& myTag, const OBJ_TAG& otherTag) override;
	virtual OBJ_TAG GetTag(const btCollisionObject& obj) const override;

	void CreateTrackRigidBody(btCollisionShape& shape);
	void CreateCheckpoints(btCollisionShape& shape, const std::vector<CheckpointShape::CheckpointInfo>& infos);

private:
	RigidBody mTrack;
	std::deque<RigidBody> mCheckpoints;
};