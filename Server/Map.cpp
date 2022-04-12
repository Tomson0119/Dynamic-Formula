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
		mCheckpoints.back().SetUpdateFlag(RigidBody::UPDATE_FLAG::CREATION);
	}
}

void Map::UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld)
{
	mTrack.Update(physicsWorld);
	for (auto& cp : mCheckpoints)
	{
		cp.Update(physicsWorld);
	}
}

void Map::Reset(btDiscreteDynamicsWorld* physicsWorld)
{
	mTrack.RemoveRigidBody(physicsWorld);
	for (auto& cp : mCheckpoints)
	{
		cp.RemoveRigidBody(physicsWorld);
	}
}