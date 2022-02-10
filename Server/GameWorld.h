#pragma once

#include "BPHandler.h"
#include "RigidBody.h"

class Player;
class WaitRoom;

class GameWorld
{
	using RoomList = std::array<std::unique_ptr<WaitRoom>, MAX_ROOM_SIZE>;
	using PlayerList = std::array<Player*, MAX_ROOM_CAPACITY>;

public:
	GameWorld();
	~GameWorld() = default;
	
	void InitPhysics(float gravity);
	void InitMapRigidBody(BtTerrainShape* terrainShape, const std::vector<std::unique_ptr<BtBoxShape>>& objShapes);
	void InitPlayerList(const btVector3 startPosition, btScalar offsetX, RoomList& roomList);

	void CreatePlayerRigidBody(int idx, btScalar mass, BtCarShape* shape);

public:
	const PlayerList& GetPlayerList() const { return mPlayerList; }

public:
	BPHandler mPhysics;
	MapRigidBody mMapRigidBody;

	PlayerList mPlayerList;
};