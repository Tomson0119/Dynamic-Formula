#pragma once

#include "BPHandler.h"
#include "RigidBody.h"
#include "InGameServer.h"
#include "Map.h"

class Player;
class WaitRoom;
struct OBJ_MASK;

class GameWorld
{
	using PlayerList = std::array<Player*, MAX_ROOM_CAPACITY>;
public:
	GameWorld(std::shared_ptr<InGameServer::GameConstant> constantsPtr);
	~GameWorld();
	
	void InitPhysics(float gravity);
	void InitMapRigidBody(const BtMapShape& mapShape, const CheckpointShape& cpShape);
	void InitPlayerList(WaitRoom* room, int cpCount);

	void SetPlayerTransform(int idx, const btVector3& pos, const btQuaternion& quat);

	void CreateRigidbodies(int idx,
		btScalar carMass, BtCarShape& carShape,
		btScalar missileMass, BtBoxShape& missileShape);

	void UpdatePhysicsWorld();
	void FlushPhysicsWorld();
	void RemovePlayerRigidBody(int idx);

	void HandleKeyInput(int idx, uint8_t key, bool pressed);
	bool CheckIfAllLoaded(int idx);

public:
	void SetActive(bool active);
	bool IsActive() const { return mActive; }

	const PlayerList& GetPlayerList() const { return mPlayerList; }
	WSAOVERLAPPEDEX* GetPhysicsOverlapped();

private:
	void CheckCollision();
	void UpdatePlayers(float elapsed);

	void HandleCollision(
		const btCollisionObject& objA,
		const btCollisionObject& objB,
		GameObject& gameObjA, 
		GameObject& gameObjB);

	void HandleCollisionWithMap(int idx, int cpIdx,	int mask);
	void HandleCollisionWithPlayer(int aIdx, int bIdx, int aMask, int bMask);

	void HandleInvincibleMode(int idx);
	void SpawnToCheckpoint(Player& player);
	int GetPlayerIndex(const GameObject& obj);

public:
	void SendGameStartSuccess();
	void SendStartSignal();

private:
	void BroadcastAllTransform();
	void PushVehicleTransformPacket(int target, int receiver);
	void PushMissileTransformPacket(int target, int receiver);
	void SendMissileRemovePacket(int target);
	void SendInvincibleOnPacket(int target);
	void SendSpawnPacket(int target);
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);
	
	//TEST
	void TestVehicleSpawn();

private:
	int mID;
	WSAOVERLAPPEDEX mPhysicsOverlapped;

	std::atomic_int mPlayerCount;
	std::atomic_bool mActive;

	// TEST
	std::atomic_bool mTestFlag = false;

	int mUpdateTick;
	
	Map mMap;
	PlayerList mPlayerList;

	BPHandler mPhysics;
	class Timer mTimer;

	std::shared_ptr<InGameServer::GameConstant> mConstantPtr;
};