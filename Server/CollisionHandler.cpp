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

		GameObject* playerA = reinterpret_cast<GameObject*>(objA->getUserPointer());
		GameObject* playerB = reinterpret_cast<GameObject*>(objB->getUserPointer());

		auto aTag = GetTag(objA, playerA);
		auto bTag = GetTag(objB, playerB);

		if (aTag == OBJ_TAG::VEHICLE) // handles (vehicle vs missile)
		{
		}
		else if (playerA) // handles (vehicle vs checkpoints) || (missile vs track)
		{

		}
		else if (playerB) // handles (vehicle vs checkpoints) || (missile vs track)
		{

		}
	}
}

OBJ_TAG CollisionHandler::GetTag(const btCollisionObject* obj, GameObject* gameObj)
{
	if (obj == nullptr || gameObj == nullptr)
	{
		return OBJ_TAG::NONE;
	}
	return OBJ_TAG::NONE;
}
