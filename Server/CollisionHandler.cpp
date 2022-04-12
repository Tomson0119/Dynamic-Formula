#include "common.h"
#include "CollisionHandler.h"
#include "Player.h"
#include "Map.h"
#include "GameWorld.h"

CollisionHandler& CollisionHandler::GetInstance()
{
	static CollisionHandler instance;
	return instance;
}

void CollisionHandler::CheckCollision(btDiscreteDynamicsWorld& dynamicsWorld, GameWorld& gameWorld)
{
	int numManifolds = dynamicsWorld.getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = 
			dynamicsWorld.getDispatcher()->getManifoldByIndexInternal(i);
		
		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();

		CollisionObject* gameObjA = reinterpret_cast<CollisionObject*>(objA->getUserPointer());
		CollisionObject* gameObjB = reinterpret_cast<CollisionObject*>(objB->getUserPointer());

		auto aTag = gameObjA->GetTag(*objA);
		auto bTag = gameObjB->GetTag(*objB);

		gameObjA->HandleCollisionWith(aTag, bTag);
		gameObjB->HandleCollisionWith(bTag, aTag);
	}
}