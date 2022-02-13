#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "RigidBody.h"

GameWorld::GameWorld()
	: mID{ -1 }, mActive{ false },
	  mPlayerCount{ 0 },
	  mPhysicsOverlapped{ OP::PHYSICS }
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
	mMapRigidBody.CreateTerrainRigidBody(terrainShape);
	// TODO: CreateStaticObjectRigidBodies;
}

void GameWorld::InitPlayerList(const btVector3 startPosition, btScalar offsetX, WaitRoom* room)
{
	mID = room->GetID();

	for (int i = 0; auto& player : mPlayerList)
	{
		player = room->GetPlayerPtr(i);
		player->SetPosition(
			startPosition.x() + offsetX * i,
			startPosition.y(),
			startPosition.z());
		i++;
	}
}

void GameWorld::CreatePlayerRigidBody(int idx, btScalar mass, BtCarShape* shape)
{
	mPlayerList[idx]->CreateVehicleRigidBody(mass, mPhysics.GetDynamicsWorld(), shape);
	mPlayerCount += 1;
}

void GameWorld::UpdatePhysicsWorld(float timeStep)
{
	for (Player* player : GetPlayerList())
		player->UpdatePlayerRigidBody(mPhysics.GetDynamicsWorld());

	mMapRigidBody.UpdateAllRigidBody(mPhysics.GetDynamicsWorld());

	mPhysics.StepSimulation(timeStep);
}

void GameWorld::FlushPhysicsWorld()
{
	mPhysics.Flush();
}

void GameWorld::RemovePlayerRigidBody(int idx)
{
	mPlayerList[idx]->SetDeletionFlag();
	mPlayerCount -= 1;

	if (mPlayerCount == 0)
		SetActive(false);
}

void GameWorld::SendGameStartSuccess()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send game start packet.\n";
#endif
	SC::packet_game_start_success info_pck{};
	info_pck.size = sizeof(SC::packet_game_start_success);
	info_pck.type = SC::GAME_START_SUCCESS;
	info_pck.room_id = mID;

	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		const btVector3 pos = mPlayerList[i]->GetPosition();
		info_pck.x[i] = pos.x();
		info_pck.y[i] = pos.y();
		info_pck.z[i] = pos.z();
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size);
}

WSAOVERLAPPEDEX* GameWorld::GetOverlapped(float timeStep)
{
	mPhysicsOverlapped.Reset(OP::PHYSICS, reinterpret_cast<std::byte*>(&timeStep), sizeof(timeStep));
	return &mPhysicsOverlapped;
}

void GameWorld::SendToAllPlayer(std::byte* pck, int size, int ignore, bool instSend)
{
	for (const auto& player : mPlayerList)
	{
		if (player->Empty == false && player->ID != ignore)
		{
			gClients[player->ID]->PushPacket(pck, size);
			if (instSend) gClients[player->ID]->SendMsg();
		}
	}
}
