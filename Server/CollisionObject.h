#pragma once

class CollisionObject
{
protected:
	enum class OBJ_TAG : uint8_t
	{
		NONE = 0,
		VEHICLE,
		MISSILE,
		TRACK,
		CHECKPOINT
	};

public:
	virtual ~CollisionObject() = default;
	virtual void UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld) = 0;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) = 0;
	virtual void HandleCollisionWith(const OBJ_TAG& myTag, const OBJ_TAG& otherTag) = 0;
	virtual OBJ_TAG GetTag(const btCollisionObject& obj) const = 0;
};