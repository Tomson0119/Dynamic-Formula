#include "stdafx.h"
#include "roomScene.h"
#include "NetLib/NetModule.h"

RoomScene::RoomScene(NetModule* netPtr)
	: Scene{ SCENE_STAT::ROOM, (XMFLOAT4)Colors::Chocolate, netPtr }
{
	OutputDebugStringW(L"Room Scene Entered.\n");
#ifdef STANDALONE
	SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
#elif defined(START_GAME_INSTANT)
	mStartTime = std::chrono::high_resolution_clock::now();
	mNetPtr->Client()->ToggleReady(mNetPtr->GetRoomID());
	mSendFlag = false;
#endif
}

void RoomScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<BulletWrapper> physics)
{
}

void RoomScene::Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<BulletWrapper> physics)
{
	// TEST
#ifdef START_GAME_INSTANT
	// send start packet again until game actually start
	auto currTime = std::chrono::high_resolution_clock::now();
	if ((currTime - mStartTime) > 1000ms && mSendFlag) {
		mNetPtr->Client()->ToggleReady(mNetPtr->GetRoomID());
		mSendFlag = false;
	}
#endif
}

void RoomScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer)
{
}

bool RoomScene::ProcessPacket(std::byte* packet, char type, int bytes)
{
	switch (type)
	{	
	case SC::ROOM_INSIDE_INFO:
	{
		OutputDebugString(L"Received room inside info packet.\n");
		SC::packet_room_inside_info* pck = reinterpret_cast<SC::packet_room_inside_info*>(packet);
		mNetPtr->InitRoomInfo(pck);
		break;
	}
	case SC::UPDATE_PLAYER_INFO:
	{
		OutputDebugString(L"Received update player info packet.\n");
		SC::packet_update_player_info* pck = reinterpret_cast<SC::packet_update_player_info*>(packet);
		mNetPtr->UpdatePlayerInfo(pck);
		break;
	}
	case SC::UPDATE_MAP_INFO:
	{
		OutputDebugString(L"Received update map info packet.\n");
		SC::packet_update_map_info* pck = reinterpret_cast<SC::packet_update_map_info*>(packet);
		mNetPtr->UpdateMapIndex(pck);
		break;
	}
	case SC::REMOVE_PLAYER:
	{
		OutputDebugString(L"Received remove player packet.\n");

		SC::packet_remove_player* pck = reinterpret_cast<SC::packet_remove_player*>(packet);
		mNetPtr->RemovePlayer(pck);
		break;
	}
	case SC::GAME_START_SUCCESS:
	{
		OutputDebugString(L"Received game start packet.\n");

		SC::packet_game_start_success* pck = reinterpret_cast<SC::packet_game_start_success*>(packet);
		if (pck->room_id == mNetPtr->GetRoomID())
		{
			mNetPtr->InitPlayersPosition(pck);
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		}
		break;
	}
	case SC::GAME_START_FAIL:
	{		
		SC::packet_game_start_fail* pck = reinterpret_cast<SC::packet_game_start_fail*>(packet);
		if (pck->room_id == mNetPtr->GetRoomID())
		{
		#ifdef START_GAME_INSTANT
			mStartTime = std::chrono::high_resolution_clock::now();
			mSendFlag = true;	
		#endif
		}
		break;
	}
	case SC::FORCE_LOGOUT:
	{
		OutputDebugString(L"Received force logout packet.\n");
		SetSceneChangeFlag(SCENE_CHANGE_FLAG::LOGOUT);
		break;
	}
	default:
		OutputDebugString(L"Invalid Packet.\n");
		return false;
	}	
	return true;
}