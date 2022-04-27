#include "common.h"
#include "Map.h"
#include "ObjectMask.h"

Map::Map()
{
}

void Map::CreateTrackRigidBody(btCollisionShape& shape)
{
	mTrack.SetMaskBits(OBJ_MASK_GROUP::TRACK, OBJ_MASK::TRACK);
	mTrack.CreateRigidBody(0.0f, shape, this);
	mTrack.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATE);
}

void Map::CreateCheckpoints(btCollisionShape& shape, const std::vector<CheckpointShape::CheckpointInfo>& infos)
{
	for (const auto& info : infos)
	{
		mCheckpoints.emplace_back();
		mCheckpoints.back().SetTransform(info.position, info.rotation);
		mCheckpoints.back().SetMaskBits(OBJ_MASK_GROUP::CHECKPOINT, OBJ_MASK::CHECKPOINT);
		mCheckpoints.back().CreateRigidBody(0.0f, shape, this);
		mCheckpoints.back().SetNoResponseCollision();
		mCheckpoints.back().SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATE);
	}
}

void Map::Update(float elapsed, BPHandler& physics)
{
	mTrack.Update(physics);
	for (auto& cp : mCheckpoints)
	{
		cp.Update(physics);
	}
}

void Map::Reset(BPHandler& physics)
{
	mTrack.RemoveRigidBody(physics);
	mTrack.Flush();
	for (auto& cp : mCheckpoints)
	{
		cp.RemoveRigidBody(physics);
		cp.Flush();
	}
}

int Map::GetMask(const btCollisionObject& obj) const
{
	if (&obj == mTrack.GetRigidBody())
	{
		return OBJ_MASK::TRACK;
	}
	else
	{
		return OBJ_MASK::CHECKPOINT;
	}
}

int Map::GetCheckpointIndex(const btCollisionObject& obj) const
{
	for (int i = 0; const RigidBody& cp : mCheckpoints)
	{
		if (&obj == cp.GetRigidBody())
		{
			return i;
		}
		i += 1;
	}
	return -1;
}

const RigidBody& Map::GetCheckpointRigidBody(int idx) const
{
	Helper::Assert((0 <= idx && idx < mCheckpoints.size()), "Checkpoint index out of bound.");
	return mCheckpoints[idx];
}
