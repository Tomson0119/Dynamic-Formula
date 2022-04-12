#pragma once



class CollisionObject;
class GameWorld;

class CollisionHandler
{
public:
	static CollisionHandler& GetInstance();
	static void CheckCollision(btDiscreteDynamicsWorld& dynamicsWorld, GameWorld& gameWorld);
};