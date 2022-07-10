#include "../stdafx.h"

#include "../scene.h"
#include "../loginScene.h"
#include "../lobbyScene.h"
#include "../roomScene.h"
#include "../inGameScene.h"

#include "NetModule.h"

NetModule::NetModule()
	: mLoop{ true }, mScenePtr{ nullptr }, mRoomID{ -1 },
	  mAdminIdx{ -1 }, mPlayerIdx{ -1 }, mMapIdx{ -1 },
	  mLatencyMs{ 0 }
{
	for (int i = 0; i < mPlayerList.size(); i++)
		mPlayerList[i] = PlayerInfo{ true, -1, false, "", XMFLOAT3{ 0.0f,0.0f,0.0f } };

	mNetClient = std::make_unique<NetClient>();
	MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Init(10);
}

NetModule::~NetModule()
{
	if (mHolePunchingThread.joinable()) mHolePunchingThread.join();
	if (mNetThread.joinable()) mNetThread.join();
}

void NetModule::StarttHolePunching()
{
	mHolePunchingThread = std::thread{ HolePunchingFunc, std::ref(*this) };
}

bool NetModule::Connect(const std::string& ip, u_short port)
{
	if (mNetClient->Connect(ip, port))
	{
		Init();	
		return true;
	}
	return false;
}

void NetModule::PostDisconnect()
{	
	if (mNetClient->IsConnected())
	{
		mIOCP.PostToCompletionQueue(nullptr, -1);
		OutputDebugStringA("Posting disconnection operation.\n");
	}
}

void NetModule::NetworkFunc(NetModule& net)
{
	CompletionInfo info{};
	while (net.mLoop)
	{
		try {
			net.mIOCP.GetCompletionInfo(info);

			int id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);
			
			if (over_ex == nullptr)
			{
				net.mLoop = false;
				net.mNetClient->Disconnect();
				continue;
			}
			if (info.success == FALSE)
			{
				if (id == 0) net.PostDisconnect();
				if (over_ex && over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
			}
			net.HandleCompletionInfo(over_ex, info.bytes, id);
		}
		catch (std::exception& ex)
		{
			std::string str = ex.what();
			OutputDebugStringA((str + "\n").c_str());
		}
	}
}

void NetModule::HolePunchingFunc(NetModule& net)
{
	while (net.IsHolePunchingDone() == false)
	{
		net.Client()->SendUDPConnectionPacket();
		std::this_thread::sleep_for(20ms);
	}
	Log::Print("Hole punching thread exit.");
}

void NetModule::InitRoomInfo(SC::packet_room_inside_info* pck)
{
	mAdminIdx = pck->admin_idx;
	mMapIdx = pck->map_id;
	mPlayerIdx = pck->player_idx;
	mPlayerListMut.lock();
	for (int i = 0; i < mPlayerList.size(); i++)
	{
		mPlayerList[i].Empty = pck->player_stats[i].empty;
		mPlayerList[i].Color = pck->player_stats[i].color;
		mPlayerList[i].Ready = pck->player_stats[i].ready;
		strncpy_s(mPlayerList[i].Name, pck->player_stats[i].name, MAX_NAME_SIZE - 1);
	}
	mPlayerListMut.unlock();
}

void NetModule::RemovePlayer(SC::packet_remove_player* pck)
{	
	mAdminIdx = pck->admin_idx;
	mPlayerListMut.lock();
	mPlayerList[pck->player_idx].Empty = true;
	mPlayerListMut.unlock();
}

void NetModule::UpdateMapIndex(SC::packet_update_map_info* pck)
{
	mMapIdx = pck->map_id;
}

void NetModule::UpdatePlayerInfo(SC::packet_update_player_info* pck)
{
	mAdminIdx = pck->admin_idx;

	mPlayerListMut.lock();
	PlayerInfo& info = mPlayerList[pck->player_idx];
	info.Empty = pck->player_info.empty;
	info.Color = pck->player_info.color;
	info.Ready = pck->player_info.ready;
	strncpy_s(info.Name, pck->player_info.name, MAX_NAME_SIZE - 1);
	mPlayerListMut.unlock();
}

void NetModule::UpdateRoomList(SC::packet_room_outside_info* pck)
{
	mRoomListMut.lock();
	for (int i = 0; i < ROOM_NUM_PER_PAGE; i++)
	{
		mRoomList[i].ID = pck->rooms[i].room_id;
		mRoomList[i].PlayerCount = pck->rooms[i].player_count;
		mRoomList[i].MapID = pck->rooms[i].map_id;
		mRoomList[i].GameStarted = pck->rooms[i].game_started;
		mRoomList[i].Opened = pck->rooms[i].room_opened;
	}
	mRoomListMut.unlock();
}

void NetModule::InitPlayerTransform(SC::packet_game_start_success* pck)
{
	mPlayerListMut.lock();
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		auto& pos = mPlayerList[i].StartPosition;
		auto& quat = mPlayerList[i].StartRotation;

		pos.x = pck->positions[i].x / POS_FLOAT_PRECISION;
		pos.y = pck->positions[i].y / POS_FLOAT_PRECISION;
		pos.z = pck->positions[i].z / POS_FLOAT_PRECISION;

		auto elems = Compressor::DecodeQuat(pck->quaternions[i]);
		quat.x = elems[0];
		quat.y = elems[1];
		quat.z = elems[2];
		quat.w = elems[3];
	}
	mPlayerListMut.unlock();
}

void NetModule::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes, int id)
{
	switch (over->Operation)
	{
	case OP::RECV:
	{
		if (id == 0 && bytes == 0)
		{
			PostDisconnect();
			break;
		}
		over->NetBuffer.ShiftWritePtr(bytes);
		ReadRecvBuffer(over, bytes);
		mNetClient->RecvMsg(id);
		break;
	}
	case OP::SEND:
	{
		if (bytes != over->WSABuffer.len)
		{
			// NEED TEST
			//PostDisconnect();
		}
		delete over;
		break;
	}
	}
}

void NetModule::ReadRecvBuffer(WSAOVERLAPPEDEX* over, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		SC::PCK_TYPE type = static_cast<SC::PCK_TYPE>(GetPacketType(packet));

		if (packet == nullptr) {
			over->NetBuffer.Clear();
			break;
		}
		if (mScenePtr && mScenePtr->ProcessPacket(packet, type, bytes) == false) {
			over->NetBuffer.Clear();
			break;
		}
	}
}

NetModule::PlayerList NetModule::GetPlayersInfo()
{
	mPlayerListMut.lock();
	auto ret = mPlayerList;
	mPlayerListMut.unlock();
	return ret;
}

NetModule::RoomList NetModule::GetRoomList()
{
	mRoomListMut.lock();
	auto ret = mRoomList;
	mRoomListMut.unlock();
	return ret;
}

void NetModule::CalcClockDelta(uint64_t serverTimeStamp)
{
	mServerTimeStamp = ConvertNsToMs(serverTimeStamp);
	auto clientTimeStampMs = ConvertNsToMs(Clock::now().time_since_epoch().count()) - mLatencyMs;
	mClockSyncDeltaMs = mServerTimeStamp - clientTimeStampMs;
}

void NetModule::Init()
{
	mIOCP.RegisterDevice(mNetClient->GetTCPSocket(), 0);
	mNetClient->RecvMsg(false);

	mNetClient->BindUDPSocket(mNetClient->GetTCPSckPort());	
	mIOCP.RegisterDevice(mNetClient->GetUDPSocket(), 1);
	
	mNetThread = std::thread{ NetworkFunc, std::ref(*this) };
}