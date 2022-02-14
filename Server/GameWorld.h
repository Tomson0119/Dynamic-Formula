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
	void FlushPhysicsWorld();

	void RemovePlayerRigidBody(int idx);

	void SendGameStartSuccess();

public:
	void SetActive(bool active) { mActive = active; }
	bool IsActive() const { return mActive; }

	const PlayerList& GetPlayerList() const { return mPlayerList; }
	WSAOVERLAPPEDEX* GetOverlapped(float timeStep);

private:
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);

private:
	int mID;
	WSAOVERLAPPEDEX mPhysicsOverlapped;

	std::atomic_int mPlayerCount;
	std::atomic_bool mActive;

	BPHandler mPhysics;
	MapRigidBody mMapRigidBody;
	PlayerList mPlayerList;
};