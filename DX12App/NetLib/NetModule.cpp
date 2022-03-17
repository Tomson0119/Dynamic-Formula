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
	  mIsConnected{ false }, mLatency{ 0 }
{
	for (int i = 0; i < mPlayerList.size(); i++)
		mPlayerList[i] = PlayerInfo{ true, -1, false, "", XMFLOAT3{ 0.0f,0.0f,0.0f } };

	mNetClient = std::make_unique<NetClient>();
}

NetModule::~NetModule()
{
	if (mNetThread.joinable()) mNetThread.join();
}

bool NetModule::Connect(const char* ip, short port)
{
	if (mIsConnected = mNetClient->Connect(ip, port))
	{
		Init();		
		return true;
	}
	return false;
}

void NetModule::PostDisconnect()
{	
	if (mIsConnected)
	{
		// Post disconnect operation to get out of the thread loop	
		WSAOVERLAPPEDEX* over = new WSAOVERLAPPEDEX(OP::DISCONNECT);
		mIOCP.PostToCompletionQueue(over, 0);
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
				net.PostDisconnect();
				continue;
			}
			if (info.success == FALSE)
			{
				if (id == 1 && over_ex && over_ex->Operation == OP::RECV)
				{
					OutputDebugStringA("This shouldn't be called.\n");
					over_ex->NetBuffer.Clear();
					net.mNetClient->RecvMsg(true);
					continue;
				}
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

void NetModule::InitRoomInfo(SC::packet_room_inside_info* pck)
{
	if (mRoomID == pck->room_id)
	{
		mAdminIdx = pck->admin_idx;
		mMapIdx = pck->map_id;
		mPlayerIdx = pck->player_idx;
		for (int i = 0; i < mPlayerList.size(); i++)
		{
			mPlayerList[i].Empty = pck->player_stats[i].empty;
			mPlayerList[i].Color = pck->player_stats[i].color;
			mPlayerList[i].Ready = pck->player_stats[i].ready;
			strncpy_s(mPlayerList[i].Name, pck->player_stats->name, MAX_NAME_SIZE - 1);
		}
	}
}

void NetModule::RemovePlayer(SC::packet_remove_player* pck)
{
	if (mRoomID == pck->room_id)
	{
		mAdminIdx = pck->admin_idx;
		mPlayerList[pck->player_idx].Empty = true;
	}
}

void NetModule::UpdateMapIndex(SC::packet_update_map_info* pck)
{
	if (mRoomID == pck->room_id)
	{
		mMapIdx = pck->map_id;
	}
}

void NetModule::UpdatePlayerInfo(SC::packet_update_player_info* pck)
{
	if (mRoomID == pck->room_id)
	{
		mAdminIdx = pck->admin_idx;

		PlayerInfo& info = mPlayerList[pck->player_idx];
		info.Empty = pck->player_info.empty;
		info.Color = pck->player_info.color;
		info.Ready = pck->player_info.ready;
		strncpy_s(info.Name, pck->player_info.name, MAX_NAME_SIZE - 1);
	}
}

void NetModule::InitPlayersPosition(SC::packet_game_start_success* pck)
{
	if (mRoomID == pck->room_id)
	{
		for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
		{
			auto& pos = mPlayerList[i].StartPosition;
			pos.x = pck->x[i] / FIXED_FLOAT_LIMIT;
			pos.y = pck->y[i] / FIXED_FLOAT_LIMIT;
			pos.z = pck->z[i] / FIXED_FLOAT_LIMIT;
		}
	}
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
			PostDisconnect();
		delete over;
		break;
	}
	case OP::DISCONNECT:
	{
		bool b = true;
		mLoop.compare_exchange_strong(b, false);
		mNetClient->Disconnect();
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
		char type = GetPacketType(packet);

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

void NetModule::SetLatency(uint64_t sendTime)
{
	auto duration = Clock::now().time_since_epoch();
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	mLatency = now - sendTime;

	mUpdateRate = std::chrono::duration_cast<std::chrono::milliseconds>(duration - mTimeStamp).count();
	mTimeStamp = duration;
}

void NetModule::Init()
{
	mIOCP.RegisterDevice(mNetClient->GetTCPSocket(), 0);
	mNetClient->RecvMsg(false);

	mIOCP.RegisterDevice(mNetClient->GetUDPSocket(), 1);
	
	mNetThread = std::thread{ NetworkFunc, std::ref(*this) };
}
