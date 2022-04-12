#include "common.h"
#include "CollisionHandler.h"
#include "Player.h"
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

		Player* playerA = reinterpret_cast<Player*>(objA->getUserPointer());
		Player* playerB = reinterpret_cast<Player*>(objB->getUserPointer());

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

OBJ_TAG CollisionHandler::GetTag(const btCollisionObject* obj, Player* player)
{
	if (obj == nullptr || player == nullptr)
	{
		return OBJ_TAG::NONE;
	}

	if (obj == player->GetVehicleRigidBody().GetRigidBody())
	{
		return OBJ_TAG::VEHICLE;
	}
	else if (obj == player->GetMissileRigidBody().GetRigidBody())
	{
		return OBJ_TAG::MISSILE;
	}
	else
	{
		return OBJ_TAG::NONE;
	}
}
