#pragma once

#include "BPHandler.h"
#include "RigidBody.h"
#include "InGameServer.h"

class Player;
class WaitRoom;

class GameWorld
{
	using PlayerList = std::array<Player*, MAX_ROOM_CAPACITY>;
public:
	GameWorld(std::shared_ptr<InGameServer::VehicleConstant> constantsPtr);
	~GameWorld();
	
	void InitPhysics(float gravity);
	void InitMapRigidBody(BtTerrainShape* terrainShape, const std::vector<std::unique_ptr<BtBoxShape>>& objShapes);
	void InitPlayerList(WaitRoom* room);

	void SetPlayerPosition(int idx, const btVector3& pos);

	void CreateRigidbodies(int idx,
		btScalar carMass, BtCarShape* carShape,
		btScalar missileMass, BtBoxShape* missileShape);
	void UpdatePhysicsWorld();
	void FlushPhysicsWorld();

	void RemovePlayerRigidBody(int idx);

	void HandleKeyInput(int idx, uint8_t key, bool pressed);

	void SendGameStartSuccess();
	void SendStartSignal();

	void PushTransformPacket(int target, int receiver);
	void BroadcastAllTransform();

	bool CheckIfAllLoaded(int idx);

public:
	void SetActive(bool active);
	bool IsActive() const { return mActive; }

	const PlayerList& GetPlayerList() const { return mPlayerList; }
	WSAOVERLAPPEDEX* GetPhysicsOverlapped();

private:
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);

private:
	int mID;
	WSAOVERLAPPEDEX mPhysicsOverlapped;

	std::atomic_int mPlayerCount;
	std::atomic_bool mActive;

	std::atomic_int mUpdateTick;
	
	MapRigidBody mMapRigidBody;
	PlayerList mPlayerList;

	BPHandler mPhysics;
	class Timer mTimer;

	std::shared_ptr<InGameServer::VehicleConstant> mConstantPtr;
};