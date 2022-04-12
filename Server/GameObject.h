#pragma once

class GameObject
{
public:
	virtual ~GameObject() = default;
	virtual void UpdateRigidbodies(float elapsed, btDiscreteDynamicsWorld* physicsWorld) = 0;
	virtual void Reset(btDiscreteDynamicsWorld* physicsWorld) = 0;
};