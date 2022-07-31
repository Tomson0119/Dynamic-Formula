#pragma once

class BPHandler;

class GameObject
{
public:
	virtual ~GameObject() = default;
	virtual void Update(float elapsed, BPHandler& physics) = 0;
	virtual void Reset(BPHandler* physics) = 0;
	virtual int GetMask(const btCollisionObject& obj) const = 0;
};