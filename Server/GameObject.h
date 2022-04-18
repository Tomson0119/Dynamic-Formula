#pragma once

class GameObject
{
protected:
	enum class OBJ_TAG : int
	{
		NONE = 0,
		VEHICLE,
		MISSILE,
		TRACK,
		CHECKPOINT
	};

public:
	virtual ~GameObject() = default;
	virtual void Update(float elapsed, btDiscreteDynamicsWorld* physicsWorld) = 0;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) = 0;
	virtual void HandleCollisionWith(const btCollisionObject& objA, const btCollisionObject& objB, GameObject& otherObj) = 0;
	virtual OBJ_TAG GetTag(const btCollisionObject& obj) const = 0;
};