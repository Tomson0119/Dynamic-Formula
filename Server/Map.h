#pragma once

#include "GameObject.h"
#include "RigidBody.h"

class Map : public GameObject
{
public:
	Map();
	virtual ~Map() = default;

	virtual void Update(float elapsed, BPHandler& physics) override;
	virtual void Reset(BPHandler& physics) override;
	virtual int GetMask(const btCollisionObject& obj) const override;

	int GetCheckpointIndex(const btCollisionObject& obj) const;
	const RigidBody& GetCheckpointRigidBody(int idx) const;

	void CreateTrackRigidBody(btCollisionShape& shape);
	void CreateCheckpoints(btCollisionShape& shape, const std::vector<CheckpointShape::CheckpointInfo>& infos);

public:
	size_t GetCheckpointSize() const { return mCheckpoints.size(); }

private:
	RigidBody mTrack;
	std::deque<RigidBody> mCheckpoints;
};