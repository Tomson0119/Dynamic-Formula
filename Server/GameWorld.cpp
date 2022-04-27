#include "common.h"
#include "GameWorld.h"
#include "WaitRoom.h"
#include "Player.h"
#include "Client.h"
#include "LoginServer.h"
#include "InGameServer.h"
#include "RigidBody.h"
#include "ObjectMask.h"

GameWorld::GameWorld(std::shared_ptr<GameConstant> constantPtr)
	: mID{ -1 }, 
	  mUpdateTick{ 0 },
	  mActive{ false },
	  mPlayerCount{ 0 }, 
	  mPhysicsOverlapped{ OP::PHYSICS }
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

void GameWorld::InitMapRigidBody(const BtMapShape& mapShape, const CheckpointShape& cpShape)
{
	mMap.CreateTrackRigidBody(mapShape.GetCompoundShape());
	mMap.CreateCheckpoints(cpShape.GetCollisionShape(), cpShape.GetInfos());
}

void GameWorld::InitPlayerList(WaitRoom* room, int cpCount)
{
	mID = room->GetID();

	for (int i = 0; auto& player : mPlayerList)
	{
		player = room->GetPlayerPtr(i);
		player->SetGameConstant(mConstantPtr);
		player->SetCheckpointCount(cpCount);
		i++;
	}
}

void GameWorld::SetFinishTime(std::chrono::seconds sec)
{
	mFinishTime = Clock::now() + sec;
}

void GameWorld::SetPlayerTransform(int idx, const btVector3& pos, const btQuaternion& quat)
{
	mPlayerList[idx]->SetTransform(pos, quat);
}

void GameWorld::CreateRigidbodies(int idx,
	btScalar carMass, BtCarShape& carShape,
	btScalar missileMass, BtBoxShape& missileShape)
{
	mPlayerList[idx]->CreateVehicleRigidBody(carMass, mPhysics, carShape);
	mPlayerList[idx]->CreateMissileRigidBody(missileMass, missileShape);
	mPlayerCount += 1;
}

void GameWorld::UpdatePhysicsWorld()
{
	mTimer.Tick();
	float elapsed = mTimer.GetElapsed();

	if (elapsed > 0.0f)
	{
		mPhysics.StepSimulation(elapsed);
		CheckRunningTime(elapsed);
		CheckCollision();

		UpdatePlayers(elapsed);
		mMap.Update(elapsed, mPhysics);

	}
	mUpdateTick += 1;
	if (mUpdateTick == 2)
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
				SendItemIncreasePacket(i);
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
			// TODO: need test if client calculation matches servers
			player.ReleaseInvincible();
			player.ChangeVehicleMaskGroup(OBJ_MASK_GROUP::VEHICLE, mPhysics);
		}
	}
}

void GameWorld::FlushPhysicsWorld()
{
	for (Player* player : GetPlayerList())
	{
		if(player) player->Reset(mPhysics);
	}
	mMap.Reset(mPhysics);
	mPhysics.Flush();
}

void GameWorld::RemovePlayerRigidBody(int idx)
{
	mPlayerList[idx]->SetRemoveFlag();
	mPlayerCount -= 1;

	if (mPlayerCount == 0)
		SetActive(false);
}

void GameWorld::HandleKeyInput(int idx, uint8_t key, bool pressed)
{
	switch (static_cast<int>(key))
	{
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_LSHIFT:
	case 'Z': // boost
	case 'X': // missile.
		mPlayerList[idx]->ToggleKeyValue(key, pressed);
		break;

	case 'Q': // TEST
	{
		bool b = false;
		mTestFlag.compare_exchange_strong(b, true);
		break;
	}
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
	info_pck.type = SC::GAME_START_SUCCESS;
	info_pck.room_id = mID;

	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		const btVector3& pos = mPlayerList[i]->GetVehicleRigidBody().GetPosition();
		const btQuaternion& quat = mPlayerList[i]->GetVehicleRigidBody().GetRotation();

		info_pck.px[i] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
		info_pck.py[i] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
		info_pck.pz[i] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

		info_pck.rx[i] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
		info_pck.ry[i] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
		info_pck.rz[i] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
		info_pck.rw[i] = (int)(quat.w() * FIXED_FLOAT_LIMIT);
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&info_pck), info_pck.size);
}

void GameWorld::SendStartSignal()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Sending start signal packet.\n";
#endif
	SC::packet_start_signal pck{};
	pck.size = sizeof(SC::packet_start_signal);
	pck.type = SC::START_SIGNAL;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::BroadcastAllTransform()
{
	for (int receiver = 0; receiver < mPlayerList.size(); receiver++)
	{
		if (mPlayerList[receiver]->Empty) continue;

		for (int target = 0; target < mPlayerList.size(); target++)
		{
			if (mPlayerList[target]->Empty == false)
			{
				PushVehicleTransformPacket(target, receiver);
				PushMissileTransformPacket(target, receiver);
			}
		}
		PushDriftGaugePacket(receiver);

		int id = mPlayerList[receiver]->ID;
		if (id < 0) continue;
		gClients[id]->SendMsg(true);
	}
}

void GameWorld::PushVehicleTransformPacket(int target, int receiver)
{
	SC::packet_player_transform pck{};
	pck.size = sizeof(SC::packet_player_transform);
	pck.type = SC::PLAYER_TRANSFORM;
	pck.player_idx = target;

	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetRotation();
	const btVector3& lvel = vehicle.GetLinearVelocity();
	const btVector3& avel = vehicle.GetAngularVelocity();

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	pck.linear_vel[0] = (int)(lvel.x() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[1] = (int)(lvel.y() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[2] = (int)(lvel.z() * FIXED_FLOAT_LIMIT);

	pck.angular_vel[0] = (int)(avel.x() * FIXED_FLOAT_LIMIT);
	pck.angular_vel[1] = (int)(avel.y() * FIXED_FLOAT_LIMIT);
	pck.angular_vel[2] = (int)(avel.z() * FIXED_FLOAT_LIMIT);

	int hostID = mPlayerList[receiver]->ID;
	if (hostID < 0) return;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

void GameWorld::PushMissileTransformPacket(int target, int receiver)
{
	if (mPlayerList[target]->CheckMissileExist() == false) return;

	SC::packet_missile_transform pck{};
	pck.size = sizeof(SC::packet_missile_transform);
	pck.type = SC::MISSILE_TRANSFORM;
	pck.missile_idx = target;

	const auto& missile = mPlayerList[target]->GetMissileRigidBody();
	const btVector3& pos = missile.GetPosition();
	const btQuaternion& quat = missile.GetRotation();
	const btVector3& lvel = missile.GetLinearVelocity();

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	pck.linear_vel[0] = (int)(lvel.x() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[1] = (int)(lvel.y() * FIXED_FLOAT_LIMIT);
	pck.linear_vel[2] = (int)(lvel.z() * FIXED_FLOAT_LIMIT);

	int hostID = mPlayerList[receiver]->ID;
	if (hostID < 0) return;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

void GameWorld::PushDriftGaugePacket(int target)
{
	SC::packet_drift_gauge pck{};
	pck.size = sizeof(SC::packet_drift_gauge);;
	pck.type = SC::DRIFT_GAUGE;
	pck.gauge = (int)(mPlayerList[target]->GetDriftGauge() * FIXED_FLOAT_LIMIT);

	int hostID = mPlayerList[target]->ID;
	if (hostID < 0) return;
	gClients[hostID]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

void GameWorld::SendMissileRemovePacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send missile remove packet. [" << target << "]\n";
#endif
	SC::packet_remove_missile pck{};
	pck.size = sizeof(SC::packet_remove_missile);
	pck.type = SC::REMOVE_MISSILE;
	pck.missile_idx = target;
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendInvincibleOnPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[room id: " << mID << "] Send invincible on packet. [" << target << "]\n";
#endif
	SC::packet_invincible_on pck{};
	pck.size = sizeof(SC::packet_invincible_on);
	pck.type = SC::INVINCIBLE_ON;
	pck.player_idx = target;
	pck.duration = (int)(mConstantPtr->InvincibleDuration * FIXED_FLOAT_LIMIT);
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendSpawnPacket(int target)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send spawn packet. [" << target << "]\n";
#endif
	SC::packet_spawn_transform pck{};
	pck.size = sizeof(SC::packet_spawn_transform);
	pck.type = SC::SPAWN_TRANSFORM;
	pck.player_idx = target;
	
	const auto& vehicle = mPlayerList[target]->GetVehicleRigidBody();
	const btVector3& pos = vehicle.GetPosition();
	const btQuaternion& quat = vehicle.GetRotation();

	pck.position[0] = (int)(pos.x() * FIXED_FLOAT_LIMIT);
	pck.position[1] = (int)(pos.y() * FIXED_FLOAT_LIMIT);
	pck.position[2] = (int)(pos.z() * FIXED_FLOAT_LIMIT);

	pck.quaternion[0] = (int)(quat.x() * FIXED_FLOAT_LIMIT);
	pck.quaternion[1] = (int)(quat.y() * FIXED_FLOAT_LIMIT);
	pck.quaternion[2] = (int)(quat.z() * FIXED_FLOAT_LIMIT);
	pck.quaternion[3] = (int)(quat.w() * FIXED_FLOAT_LIMIT);

	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendWarningMsgPacket(int target, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send warning message to player[" << target << "]\n";
#endif
	SC::packet_warning_message pck{};
	pck.size = sizeof(SC::packet_warning_message);
	pck.type = SC::WARNING_MESSAGE;

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
	pck.type = SC::INGAME_INFO;
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
	pck.type = SC::GAME_END;
	
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		pck.rank[i] = mCurrRanks[i];
		pck.lap_count[i] = mPlayerList[i]->GetLapCount();
		pck.hit_count[i] = mPlayerList[i]->GetLapCount();
		pck.point[i] = mPlayerList[i]->GetPoint();
	}
	SendToAllPlayer(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GameWorld::SendItemIncreasePacket(int target, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "(room id: " << mID << ") Send game end packet.\n";
#endif
	SC::packet_item_increased pck{};
	pck.size = sizeof(SC::packet_item_increased);
	pck.type = SC::ITEM_INCREASED;
	pck.player_idx = target;

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
		// TODO: Timer needs to be singleton.
		mTimer.Start();
	}
}

WSAOVERLAPPEDEX* GameWorld::GetPhysicsOverlapped()
{
	mPhysicsOverlapped.Reset(OP::PHYSICS);
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

void GameWorld::CheckRunningTime(float elapsed)
{
	auto now = Clock::now();
	if (now >= mFinishTime)
	{
		SetActive(false);
		SendGameEndPacket();
		std::cout << "Finished\n";
	}
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

void GameWorld::HandleCollision(const btCollisionObject& objA, const btCollisionObject& objB, GameObject& gameObjA, GameObject& gameObjB)
{
	if (int idx = GetPlayerIndex(gameObjA); idx >= 0)
	{
		int aMask = gameObjA.GetMask(objA);
		int bMask = gameObjB.GetMask(objB);

		if (&gameObjB == &mMap)
			HandleCollisionWithMap(idx, mMap.GetCheckpointIndex(objB), aMask);
		else
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
				if (player->GetCurrentCPIndex() >= 0 && cpIdx == 0)
				{
					std::cout << "Lap finished.\n";
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