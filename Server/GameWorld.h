#pragma once

#include "BPHandler.h"
#include "RigidBody.h"
#include "Map.h"
#include "Timer.h"

class Player;
class WaitRoom;
struct OBJ_MASK;
struct GameConstant;

class GameWorld
{
	using PlayerList = std::array<Player*, MAX_ROOM_CAPACITY>;
public:
	GameWorld(std::shared_ptr<GameConstant> constantsPtr);
	~GameWorld() = default;
	
	void InitPhysics(float gravity);
	void InitMapRigidBody(BtMapShape& mapShape);
	void InitPlayerList(WaitRoom* room);

	void SetGameTime(std::chrono::seconds countdownSec, std::chrono::seconds finishSec);
	void SetPlayerTransform(int idx, const btVector3& pos, const btQuaternion& quat);

	void CreateRigidbodies(int idx,
		btScalar carMass, BtCarShape& carShape,
		btScalar missileMass, BtCompoundShape& missileShape);

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
	void CheckCountdownTime();
	void CheckRunningTime();
	void CheckCollision();
	void CheckMissileLaunchFlag();

	void UpdatePlayers(float elapsed);
	void UpdateInvincibleState(int idx, float elapsed);

	void HandleCollision(
		const btCollisionObject& objA,
		const btCollisionObject& objB,
		GameObject& gameObjA, 
		GameObject& gameObjB);

	void HandleCollisionWithMap(int idx, int cpIdx,	int mask);
	void HandleCollisionWithPlayer(int aIdx, int bIdx, int aMask, int bMask);

	void SetInvincibleState(int idx, float duration);
	void SpawnToCheckpoint(Player& player);
	int GetPlayerIndex(const GameObject& obj);

	void HandlePointUpdate(int target);
	void SortPlayerRanks();

public:
	void SendGameStartSuccess();
	void SendReadySignal();
	void SendStartSignal(uint64_t latency);

private:
	void BroadcastAllTransform();
	void PushVehicleTransformPacketToAll(int target);
	void PushMissileTransformPacketToAll(int target);
	void PushUiInfoPacket(int target);

	void SendMissileLaunchPacket(int target);
	void SendMissileRemovePacket(int target);
	void SendInvincibleOnPacket(int target);
	void SendSpawnPacket(int target);
	void SendWarningMsgPacket(int target, bool instSend=true);
	void SendInGameInfoPacket(int target, bool instSend=true);
	void SendGameEndPacket();
	void SendItemCountPacket(int target, bool instSend=true);

	void SendToAllPlayer(std::byte* pck, int size, bool udp=false, int ignore=-1, bool instSend=true);

private:
	int mID;
	int mUpdateTick;
	std::atomic_bool mGameStarted;
	Clock::time_point mStartTime;
	Clock::time_point mFinishTime;

	std::atomic_bool mActive;
	std::atomic_int mPlayerCount;

	WSAOVERLAPPEDEX mPhysicsOverlapped;
	
	Map mMap;
	PlayerList mPlayerList;

	BPHandler mPhysics;
	Timer mTimer;

	std::vector<int> mPrevRanks;
	std::vector<int> mCurrRanks;
	std::shared_ptr<GameConstant> mConstantPtr;

	//TEST
	Clock::time_point mSentTime;
	std::atomic_bool mManualFinish;
};