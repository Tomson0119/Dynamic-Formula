#pragma once

class BPHandler;

class GameObject
{
public:
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
	virtual void Update(float elapsed, BPHandler& physics) = 0;
	virtual void Reset(BPHandler& physics) = 0;
	virtual OBJ_TAG GetTag(const btCollisionObject& obj) const = 0;
};