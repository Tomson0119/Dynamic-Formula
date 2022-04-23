#include "common.h"
#include "Map.h"

Map::Map()
{
}

void Map::CreateTrackRigidBody(btCollisionShape& shape)
{
	mTrack.CreateRigidBody(0.0f, shape, this);
	mTrack.SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
}

void Map::CreateCheckpoints(btCollisionShape& shape, const std::vector<CheckpointShape::CheckpointInfo>& infos)
{
	for (const auto& info : infos)
	{
		mCheckpoints.emplace_back();
		mCheckpoints.back().SetPosition(info.position);
		mCheckpoints.back().SetRotation(info.rotation);
		mCheckpoints.back().CreateRigidBody(0.0f, shape, this);
		mCheckpoints.back().SetNoResponseCollision();
		mCheckpoints.back().SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
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
	for (auto& cp : mCheckpoints)
	{
		cp.RemoveRigidBody(physics);
	}
}

GameObject::OBJ_TAG Map::GetTag(const btCollisionObject& obj) const
{
	if (&obj == mTrack.GetRigidBody())
	{
		return OBJ_TAG::TRACK;
	}
	else
	{
		return OBJ_TAG::CHECKPOINT;
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
