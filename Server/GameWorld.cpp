#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "InGameServer.h"
#include "RigidBody.h"
#include "ObjectMask.h"
#include "Compressor.h"

GameWorld::GameWorld(std::shared_ptr<GameConstant> constantPtr)
	: mID{ -1 }, 
	  mUpdateTick{ 0 },
	  mGameStarted{ false },
	  mActive{ false },
	  mPlayerCount{ 0 }, 
	  mPhysicsOverlapped{ OP::PHYSICS },
	  mManualFinish{ false }
{
	mPrevRanks.resize(mPlayerList.size(), 1);
	mCurrRanks.resize(mPlayerList.size(), 1);
	mConstantPtr = constantPtr;
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		mPlayerList[i] = nullptr;
	}
}

void GameWorld::InitPhysics(float gravity)
{
	mPhysics.Init(gravity);
}

void GameWorld::InitMapRigidBody(BtMapShape& mapShape)
{
	const auto& cpShape = mapShape.GetCheckpointShape();
	mMap.CreateTrackRigidBody(mapShape.GetCompoundShape());
	mMap.CreateCheckpoints(cpShape.GetCollisionShape(), cpShape.GetInfos());
	for (int i = 0; auto & player : mPlayerList)
	{
		player->SetCheckpointCount((int)cpShape.GetInfos().size());
		i++;
	}
}

void GameWorld::InitPlayerList(WaitRoom* room)
{
	mID = room->GetID();

	for (int i = 0; auto& player : mPlayerList)
	{
		player = room->GetPlayerPtr(i);
		player->SetGameConstant(mConstantPtr);
		//player->SetCheckpointCount(cpCount);
		i++;
	}
}

void GameWorld::SetGameTime(std::chrono::seconds countdownSec, std::chrono::seconds finishSec)
{
	auto now = Clock::now();
	mStartTime = now + countdownSec;
	mFinishTime = now + finishSec + countdownSec;
}

void GameWorld::SetPlayerTransform(int idx, const btVector3& pos, const btQuaternion& quat)
{
	mPlayerList[idx]->SetTransform(pos, quat);
}

void GameWorld::CreateRigidbodies(int idx,
	btScalar carMass, BtCarShape& carShape,
	btScalar missileMass, BtCompoundShape& missileShape)
{
	mPlayerList[idx]->CreateVehicleRigidBody(carMass, mPhysics, carShape);
	mPlayerList[idx]->CreateMissileRigidBody(missileMass, missileShape);
	mPlayerCount += 1;
}

void GameWorld::UpdatePhysicsWorld()
{
	mTimer.Tick();
	float elapsed = mTimer.GetElapsed();
	
	if (mGameStarted == false)
	{
		CheckCountdownTime();
	}
	if (elapsed > 0.0f)
	{
		if (CheckRunningTime()) return;
		mPhysics.StepSimulation(elapsed);
		CheckCollision();

		UpdatePlayers(elapsed);
		mMap.Update(elapsed, mPhysics);
	}
	mUpdateTick += 1;
	if (mUpdateTick == 1)
	{
		BroadcastAllTransform();
		mUpdateTick = 0;
	}
}

void GameWorld::UpdatePlayers(float elapsed)
{
	for (int i=0; Player* player : GetPlayerList())
	{
		if (player->NeedUpdate())
		{
			player->Update(elapsed, mPhysics);
			UpdateInvincibleState(i, elapsed);
			if (player->NeedManualRespawn())
			{
				player->ResetManualRespawnFlag();
				SetInvincibleState(i, mConstantPtr->SpawnInterval);
			}
			if (player->ItemIncreased())
			{
				player->ResetItemFlag();
				SendItemCountPacket(i);
			}
			if (player->MissileTimeOver())
			{
				player->DisableMissile();
				SendMissileRemovePacket(i);
			}
		}
		i += 1;
	}
}

void GameWorld::UpdateInvincibleState(int idx, float elapsed)
{
	Player& player = *mPlayerList[idx];
	if (player.IsInvincible())
	{
		player.UpdateInvincibleDuration(elapsed);
		float duration = player.GetInvincibleDuration();

		if (player.IsActive() == false
			&& duration <= mConstantPtr->SpawnInterval)
		{
			player.Activate();
			player.StopVehicle();
			SpawnToCheckpoint(player);
			SendSpawnPacket(idx);
		}
		if (duration <= 0.0f)
		{
			player.ReleaseInvincible();
			player.ChangeVehicleMaskGroup(OBJ_MASK_GROUP::VEHICLE, mPhysics);
		}
	}
}

void GameWorld::FlushPhysicsWorld()
{
	mGameStarted = false;
	for (Player* player : GetPlayerList())
	{
		if (player)
		{
			player->Reset(&mPhysics);
		}
	}
	mMap.Reset(&mPhysics);
	mPhysics.Flush();
}

void GameWorld::RemovePlayerRigidBody(int idx)
{
	if (IsActive())
	{
		mPlayerList[idx]->SetRemoveFlag();
		mPlayerCount -= 1;

		if (mPlayerCount == 0)
			SetActive(false);
	}
}

void GameWorld::HandleKeyInput(int idx, uint8_t key, bool pressed)
{
	if (mGameStarted == false) return;

	int k = static_cast<int>(key);
	switch (k)
	{
	case VK_LCONTROL:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_LSHIFT:
	case 'Z': // boost
	case 'X': // missile.
	case 'P': // respwan
	{
		mPlayerList[idx]->ToggleKeyValue(key, pressed);
		break;
	}
	case VK_F10:
		if (pressed == false)
		{
			mManualFinish = true;
		}
		break;

	default:
		std::cout << "Invalid key input.\n";
		return;
	}
}

void GameWorld::SendGameStartSuccess()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send game start packet.\n";
#endif
	SC::packet_game_start_success info_pck{};
	info_pck.size = sizeof(SC::packet_game_start_success);
	info_pck.type = static_cast<uint8_t>(SC::PCK_TYPE::GAME_START_SUCCESS);

	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		const btVector3& pos = mPlayerList[i]->GetVehicleRigidBody().GetPosition();
		const btQuaternion& quat = mPlayerList[i]->GetVehicleRigidBody().GetRotation();

		info_pck.positions[i] = Compressor::EncodePos(pos.x(), pos.y(), pos.z());
		info_pck.quaternions[i] = Compressor::EncodeQuat(quat.x(), quat.y(), quat.z(), quat.w());
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size);
}

void GameWorld::SendReadySignal()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending ready signal packet.\n";
#endif
	SC::packet_ready_signal pck{};
	pck.size = sizeof(SC::packet_ready_signal);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::READY_SIGNAL);
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendStartSignal(uint64_t latency)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending start signal packet.\n";
#endif
	SC::packet_start_signal pck{};
	pck.size = sizeof(SC::packet_start_signal);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::START_SIGNAL);
	pck.running_time_sec = (int)mConstantPtr->GameRunningTime.count();
	pck.delay_time_msec = (int)latency;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::BroadcastAllTransform()
{
	CheckMissileLaunchFlag();

	for (int target = 0; target < mPlayerList.size(); target++)
	{
		if (mPlayerList[target]->Empty == false)
		{
			int id = mPlayerList[target]->ID;
			gClients[id]->SendMeasureRTTPacket(true, false);

			PushVehicleTransformPacketToAll(target);
			PushMissileTransformPacketToAll(target);
		}
	}
	for (int receiver = 0; receiver < mPlayerList.size(); receiver++)
	{
		if (mPlayerList[receiver]->Empty == false)
		{
			PushUiInfoPacket(receiver);
			
			int id = mPlayerList[receiver]->ID;
			if (id < 0) continue;
			gClients[id]->SendMsg(true);
		}
	}
}

void GameWorld::PushVehicleTransformPacketToAll(int target)
{
	SC::packet_player_transform pck{};
	pck.size = sizeof(SC::packet_player_transform);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::PLAYER_TRANSFORM);
	pck.player_idx = target;

	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetRotation();
	const btVector3& lvel = vehicle.GetLinearVelocity();
	const btVector3& avel = vehicle.GetAngularVelocity();

	pck.position = Compressor::EncodePos(pos.x(), pos.y(), pos.z());
	pck.quaternion = Compressor::EncodeQuat(quat.x(), quat.y(), quat.z(), quat.w());

	pck.linear_vel[0] = (int)(lvel.x() * QUAT_FLOAT_PRECISION);
	pck.linear_vel[1] = (int)(lvel.y() * QUAT_FLOAT_PRECISION);
	pck.linear_vel[2] = (int)(lvel.z() * QUAT_FLOAT_PRECISION);

	pck.speed = (int)(round(mPlayerList[target]->GetCurrentSpeed()));

	pck.time_stamp = ConvertNsToMs(Clock::now().time_since_epoch().count());

	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, true, -1, false);
}

void GameWorld::PushMissileTransformPacketToAll(int target)
{
	if (mPlayerList[target]->CheckMissileExist() == false) return;
	if (mPlayerList[target]->GetMissileLaunchFlag()) return;

	SC::packet_missile_transform pck{};
	pck.size = sizeof(SC::packet_missile_transform);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::MISSILE_TRANSFORM);
	pck.missile_idx = target;

	const auto& missile = mPlayerList[target]->GetMissileRigidBody();
	const btVector3& pos = missile.GetPosition();
	const btQuaternion& quat = missile.GetRotation();
	const btVector3& lvel = missile.GetLinearVelocity();

	pck.pos_x = (int)(pos.x() * POS_FLOAT_PRECISION);
	pck.pos_z = (int)(pos.z() * POS_FLOAT_PRECISION);

	pck.linear_vel_x = (int)(lvel.x() * POS_FLOAT_PRECISION);
	pck.linear_vel_z = (int)(lvel.z() * POS_FLOAT_PRECISION);

	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size, true, -1, false);
}

void GameWorld::PushUiInfoPacket(int target)
{
	SC::packet_ui_info pck{};
	pck.size = sizeof(SC::packet_ui_info);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::UI_INFO);
	pck.gauge = (int)(mPlayerList[target]->GetDriftGauge() * 100.0f);

	int hostID = mPlayerList[target]->ID;
	if (hostID < 0) return;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

void GameWorld::SendMissileLaunchPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send missile launch packet. [" << target << "]\n";
#endif
	SC::packet_missile_launched pck{};
	pck.size = sizeof(SC::packet_missile_launched);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::MISSILE_LAUNCHED);
	pck.missile_idx = target;

	// send vehicle tranform since missile is not updated.
	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetRotation();

	pck.position = Compressor::EncodePos(pos.x(), pos.y(), pos.z());
	pck.quaternion = Compressor::EncodeQuat(quat.x(), quat.y(), quat.z(), quat.w());

	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendMissileRemovePacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send missile remove packet. [" << target << "]\n";
#endif
	SC::packet_remove_missile pck{};
	pck.size = sizeof(SC::packet_remove_missile);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::REMOVE_MISSILE);
	pck.missile_idx = target;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendInvincibleOnPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send invincible on packet. [" << target << "]\n";
#endif
	SC::packet_invincible_on pck{};
	pck.size = sizeof(SC::packet_invincible_on);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::INVINCIBLE_ON);
	pck.player_idx = target;
	pck.duration = (int)(mConstantPtr->InvincibleDuration * 10.0f);
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendSpawnPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send spawn packet. [" << target << "]\n";
#endif
	SC::packet_spawn_transform pck{};
	pck.size = sizeof(SC::packet_spawn_transform);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::SPAWN_TRANSFORM);
	pck.player_idx = target;
	
	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetRotation();

	pck.position = Compressor::EncodePos(pos.x(), pos.y(), pos.z());
	pck.quaternion = Compressor::EncodeQuat(quat.x(), quat.y(), quat.z(), quat.z());

	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendWarningMsgPacket(int target, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send warning message to player[" << target << "]\n";
#endif
	SC::packet_warning_message pck{};
	pck.size = sizeof(SC::packet_warning_message);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::WARNING_MESSAGE);

	int id = mPlayerList[target]->ID;
	if (id < 0) return;

	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[id]->SendMsg();
}

void GameWorld::SendInGameInfoPacket(int target, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send in-game info packet [" << target << "]\n";
#endif
	SC::packet_ingame_info pck{};
	pck.size = sizeof(SC::packet_ingame_info);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::INGAME_INFO);
	pck.player_idx = target;
	pck.rank = mCurrRanks[target];
	pck.lap_count = mPlayerList[target]->GetLapCount();
	pck.point = mPlayerList[target]->GetPoint();

	int id = mPlayerList[target]->ID;
	if (id < 0) return;

	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[id]->SendMsg();
}

void GameWorld::SendGameEndPacket()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send game end packet.\n";
#endif
	SC::packet_game_end pck{};
	pck.size = sizeof(SC::packet_game_end);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::GAME_END);
	
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		pck.rank[i] = mCurrRanks[i];
		pck.lap_count[i] = mPlayerList[i]->GetLapCount();
		pck.hit_count[i] = mPlayerList[i]->GetHitCount();
		pck.point[i] = mPlayerList[i]->GetPoint();
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendItemCountPacket(int target, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send item count packet.\n";
#endif
	SC::packet_item_count pck{};
	pck.size = sizeof(SC::packet_item_count);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::ITEM_COUNT);
	pck.player_idx = target;
	pck.item_count = mPlayerList[target]->GetItemCount();

	int idx = mPlayerList[target]->ID;
	if (idx < 0) return;

	gClients[idx]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) gClients[idx]->SendMsg();
}

bool GameWorld::CheckIfAllLoaded(int idx)
{
	if (idx < 0) return false; // logic error
	mPlayerList[idx]->LoadDone = true;

	for (auto& player : mPlayerList)
	{
		if (player->Empty == false && player->LoadDone == false)
			return false;
	}
	return true;
}

void GameWorld::SetActive(bool active)
{
	mActive = active;
	if (active)
	{
		mTimer.Start();
	}
}

WSAOVERLAPPEDEX* GameWorld::GetPhysicsOverlapped()
{
	mPhysicsOverlapped.Reset(OP::PHYSICS);
	return &mPhysicsOverlapped;
}

void GameWorld::SendToAllPlayer(std::byte* pck, int size, bool udp, int ignore, bool instSend)
{
	for (const auto& player : mPlayerList)
	{
		if (player->Empty == false && player->ID != ignore)
		{
			gClients[player->ID]->PushPacket(pck, size, udp);
			if (instSend) gClients[player->ID]->SendMsg(udp);
		}
	}
}

void GameWorld::CheckCountdownTime()
{
	// not good.
	auto longestLatencyMs = [this]() -> uint64_t
	{
		uint64_t latency = 0;
		for (int i = 0; i < mPlayerList.size(); i++)
		{
			int hostID = mPlayerList[i]->ID;
			if (hostID >= 0)
			{
				latency = std::max(latency, gClients[hostID]->GetLatency());
			}
		}
		return latency;
	}();

	auto now = Clock::now();
	if (now >= (mStartTime - std::chrono::milliseconds(longestLatencyMs)))
	{
		mGameStarted = true;
		SendStartSignal(longestLatencyMs);
	}
}

bool GameWorld::CheckRunningTime()
{
	auto now = Clock::now();

	if (now >= mFinishTime || mManualFinish) // TEST for manual finish
	{
		mManualFinish = false;
		SetActive(false);
		SendGameEndPacket();
		std::cout << "Finished\n";
		return true;
	}
	return false;
}

void GameWorld::CheckCollision()
{
	int numManifolds = mPhysics.GetNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = mPhysics.GetPersistentManifold(i);

		if (contactManifold->getNumContacts() <= 0) continue;

		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();

		if (objA == nullptr || objB == nullptr) continue;

		GameObject* gameObjA = reinterpret_cast<GameObject*>(objA->getUserPointer());
		GameObject* gameObjB = reinterpret_cast<GameObject*>(objB->getUserPointer());

		HandleCollision(*objA, *objB, *gameObjA, *gameObjB);
		HandleCollision(*objB, *objA, *gameObjB, *gameObjA);
	}
}

void GameWorld::CheckMissileLaunchFlag()
{
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		if (mPlayerList[i]->Empty == false 
			&& mPlayerList[i]->GetMissileLaunchFlag())
		{
			SendMissileLaunchPacket(i);
			mPlayerList[i]->SetMissileLaunchFlag(false);
		}
	}
}

void GameWorld::HandleCollision(const btCollisionObject& objA, const btCollisionObject& objB, GameObject& gameObjA, GameObject& gameObjB)
{
	if (int idx = GetPlayerIndex(gameObjA); idx >= 0)
	{
		int aMask = gameObjA.GetMask(objA);
		int bMask = gameObjB.GetMask(objB);

		if (&gameObjB == &mMap)
			HandleCollisionWithMap(idx, mMap.GetCheckpointIndex(objB), aMask);
		else if (&gameObjA != &gameObjB)
			HandleCollisionWithPlayer(idx, GetPlayerIndex(gameObjB), aMask, bMask);
	}
}

void GameWorld::HandleCollisionWithMap(int idx, int cpIdx, int mask)
{
	switch (mask)
	{
	case OBJ_MASK::VEHICLE:
	{
		if (cpIdx >= 0)
		{
			auto player = mPlayerList[idx];
			if (player->IsNextCheckpoint(cpIdx))
			{
				std::cout << "Checkpoint: " << cpIdx << "\n";

				if (player->GetCurrentCPIndex() >= 0 && cpIdx == 0)
				{
					player->IncreaseLapCount();
					player->IncreasePoint(mConstantPtr->LapFinishPoint);
					HandlePointUpdate(idx);
				}
				player->MarkNextCheckpoint(cpIdx);
			}
			else
			{
				int cnt = player->GetReverseDriveCount(cpIdx);
				if (cnt == 1)
				{
					SendWarningMsgPacket(idx);
				}
				else if(cnt == 2)
				{
					SetInvincibleState(idx, mConstantPtr->SpawnInterval);
					player->ResetReverseCount();
				}
			}
		}
		break;
	}
	case OBJ_MASK::MISSILE:
	{
		if (cpIdx < 0)
		{
			mPlayerList[idx]->DisableMissile();
			SendMissileRemovePacket(idx);
		}
		break;
	}}
}

void GameWorld::HandleCollisionWithPlayer(int aIdx, int bIdx, int aMask, int bMask)
{
	switch (aMask)
	{
	case OBJ_MASK::VEHICLE:
	{
		if (bMask == OBJ_MASK::MISSILE)
		{
			SetInvincibleState(aIdx, mConstantPtr->InvincibleDuration);
		}
		break;
	}
	case OBJ_MASK::MISSILE:
	{
		if (bMask == OBJ_MASK::VEHICLE)
		{
			mPlayerList[aIdx]->IncreasePoint(mConstantPtr->MissileHitPoint);
			mPlayerList[aIdx]->IncreaseHitCount();
			mPlayerList[aIdx]->DisableMissile();
			SendMissileRemovePacket(aIdx);
			HandlePointUpdate(aIdx);
		}
		break;
	}}
}

void GameWorld::SetInvincibleState(int idx, float duration)
{
	SendInvincibleOnPacket(idx);

	mPlayerList[idx]->SetInvincible(duration);
	mPlayerList[idx]->ClearVehicleComponent();
	mPlayerList[idx]->ChangeVehicleMaskGroup(OBJ_MASK_GROUP::INVINCIBLE, mPhysics);
}

void GameWorld::SpawnToCheckpoint(Player& player)
{
	int currentCPIndex = player.GetCurrentCPIndex();
	if (currentCPIndex >= 0)
	{
		const RigidBody& cp = mMap.GetCheckpointRigidBody(currentCPIndex);
		player.SetTransform(cp.GetPosition(), cp.GetRotation());
	}
	else
	{
		player.SetTransform(mConstantPtr->StartPosition, mConstantPtr->StartRotation);
	}	
}

int GameWorld::GetPlayerIndex(const GameObject& obj)
{
	for (int i=0; const auto& player : mPlayerList)
	{
		if (&obj == player) return i;
		i += 1;
	}
	return -1;
}

void GameWorld::HandlePointUpdate(int target)
{
	SortPlayerRanks();
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		const auto& player = mPlayerList[i];
		if (player->Empty == false)
		{
			if (i == target || mPrevRanks[i] != mCurrRanks[i])
			{
				SendInGameInfoPacket(i);
			}
		}
	}
	mPrevRanks = mCurrRanks;
}

void GameWorld::SortPlayerRanks()
{
	std::vector<int> indexes(mPlayerList.size());
	std::iota(indexes.begin(), indexes.end(), 0);

	std::stable_sort(indexes.begin(), indexes.end(),
		[this](int a, int b)
		{
			return mPlayerList[a]->GetPoint() > mPlayerList[b]->GetPoint();
		});

	for (int i = 0, rank = 1; i < indexes.size(); i++)
	{
		if (i > 0)
		{
			int prevPoint = mPlayerList[indexes[i-1]]->GetPoint();
			int currPoint = mPlayerList[indexes[i]]->GetPoint();
			if (prevPoint > currPoint)
			{
				rank += 1;
			}
		}
		mCurrRanks[indexes[i]] = rank;
	}
}