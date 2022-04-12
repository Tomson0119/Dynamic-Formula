#pragma once

enum class OBJ_TAG : uint8_t
{
	NONE = 0,
	VEHICLE,
	MISSILE,
	TRACK
};

class Player;
class GameWorld;

class CollisionHandler
{
public:
	static CollisionHandler& GetInstance();
	static void CheckCollision(btDiscreteDynamicsWorld& dynamicsWorld, GameWorld& gameWorld);
	static OBJ_TAG GetTag(const btCollisionObject* obj, Player* player);
};