#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"

GameWorld::GameWorld()
{
	for (int i = 0; i < mPlayerList.size(); i++)
		mPlayerList[i] = nullptr;
}

void GameWorld::InitPhysics(float gravity)
{
	mPhysics.Init(gravity);
}

void GameWorld::InitMapRigidBody(BtTerrainShape* terrainShape, const std::vector<std::unique_ptr<BtBoxShape>>& objShapes)
{
	mMapRigidBody.CreateTerrainRigidBody(mPhysics.GetDynamicsWorld(), terrainShape);
	// TODO: CreateStaticObjectRigidBodies;
}

void GameWorld::InitPlayerList(const btVector3 startPosition, btScalar offsetX, RoomList& roomList)
{
	for (int i = 0; auto & player : mPlayerList)
	{
		player = roomList[i]->GetPlayerPtr(i);
		player->SetPosition(
			startPosition.x() + offsetX * i,
			startPosition.y(),
			startPosition.z());
		i++;
	}
}

void GameWorld::CreatePlayerRigidBody(int idx, btScalar mass, BtCarShape* shape)
{
	if (idx < 0) return;
	mPlayerList[idx]->CreateVehicleRigidBody(mass, mPhysics.GetDynamicsWorld(), shape);
}
