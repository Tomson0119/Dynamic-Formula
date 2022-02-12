#pragma once

#include "BPHandler.h"
#include "RigidBody.h"

class Player;
class WaitRoom;

class GameWorld
{
	using PlayerList = std::array<Player*, MAX_ROOM_CAPACITY>;

public:
	GameWorld();
	~GameWorld() = default;
	
	void InitPhysics(float gravity);
	void InitMapRigidBody(BtTerrainShape* terrainShape, const std::vector<std::unique_ptr<BtBoxShape>>& objShapes);
	void InitPlayerList(const btVector3 startPosition, btScalar offsetX, WaitRoom* room);

	void CreatePlayerRigidBody(int idx, btScalar mass, BtCarShape* shape);
	void UpdatePhysicsWorld(float timeStep);

	void RemovePlayerRigidBody(int idx);

	void SendGameStartSuccess();

public:
	const PlayerList& GetPlayerList() const { return mPlayerList; }
	WSAOVERLAPPEDEX* GetOverlapped(float timeStep);

private:
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);

public:
	int mID;
	WSAOVERLAPPEDEX mPhysicsOverlapped;

	BPHandler mPhysics;
	MapRigidBody mMapRigidBody;
	PlayerList mPlayerList;
};