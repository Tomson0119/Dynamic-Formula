#pragma once

#include "BPHandler.h"
#include "RigidBody.h"
#include "InGameServer.h"
#include "Map.h"

class Player;
class WaitRoom;

class GameWorld
{
	using PlayerList = std::array<Player*, MAX_ROOM_CAPACITY>;
public:
	GameWorld(std::shared_ptr<InGameServer::GameConstant> constantsPtr);
	~GameWorld();
	
	void InitPhysics(float gravity);
	void InitMapRigidBody(const BtMapShape& mapShape, const CheckpointShape& cpShape);
	void InitPlayerList(WaitRoom* room, int cpCount);

	void SetPlayerPosition(int idx, const btVector3& pos);
	void SetPlayerRotation(int idx, const btQuaternion& quat);

	void CreateRigidbodies(int idx,
		btScalar carMass, BtCarShape* carShape,
		btScalar missileMass, BtBoxShape* missileShape);

	void UpdatePhysicsWorld();
	void FlushPhysicsWorld();
	void RemovePlayerRigidBody(int idx);

	void HandleKeyInput(int idx, uint8_t key, bool pressed);

	void SendGameStartSuccess();
	void SendStartSignal();

	bool CheckIfAllLoaded(int idx);

public:
	void SetActive(bool active);
	bool IsActive() const { return mActive; }

	const PlayerList& GetPlayerList() const { return mPlayerList; }
	WSAOVERLAPPEDEX* GetPhysicsOverlapped();

private:
	void CheckCollision();

	void HandleCollision(
		const btCollisionObject& objA,
		const btCollisionObject& objB,
		GameObject& gameObjA, 
		GameObject& gameObjB);

	void HandleCollisionWithMap(
		int idx, int cpIdx,
		const GameObject::OBJ_TAG& tag);

	void HandleCollisionWithPlayer(
		int aIdx, int bIdx,
		const GameObject::OBJ_TAG aTag, 
		const GameObject::OBJ_TAG bTag);

	int GetPlayerIndex(const GameObject& obj);

private:
	void BroadcastAllTransform();
	void PushVehicleTransformPacket(int target, int receiver);
	void PushMissileTransformPacket(int target, int receiver);
	void SendToAllPlayer(std::byte* pck, int size, int ignore=-1, bool instSend=true);

private:
	int mID;
	WSAOVERLAPPEDEX mPhysicsOverlapped;

	std::atomic_int mPlayerCount;
	std::atomic_bool mActive;

	int mUpdateTick;
	
	Map mMap;
	PlayerList mPlayerList;

	BPHandler mPhysics;
	class Timer mTimer;

	std::shared_ptr<InGameServer::GameConstant> mConstantPtr;
};