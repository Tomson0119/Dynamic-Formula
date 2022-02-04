#include "stdafx.h"
#include "lobbyScene.h"
#include "NetLib/NetModule.h"

LobbyScene::LobbyScene(NetModule* netPtr)
	: Scene{ SCENE_STAT::LOBBY, (XMFLOAT4)Colors::Bisque, netPtr }
{
	OutputDebugStringW(L"Lobby Scene Entered.\n");
#ifdef STANDALONE
	SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
#else
	#ifdef START_GAME_INSTANT
		mNetPtr->Client()->RequestEnterRoom(0);
	#endif
#endif
}

void LobbyScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void LobbyScene::Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{

}

void LobbyScene::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer)
{
}

bool LobbyScene::ProcessPacket(std::byte* packet, char type, int bytes)
{
	switch (type)
	{
	case SC::ACCESS_ROOM_ACCEPT:
	{
		OutputDebugString(L"Received access room accept packet.\n");

		SC::packet_access_room_accept* pck = reinterpret_cast<SC::packet_access_room_accept*>(packet);
		SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		mNetPtr->SetRoomID(pck->room_id);
		break;
	}
	case SC::ACCESS_ROOM_DENY:
	{
		OutputDebugString(L"Received access room deny packet.\n");
		
		SC::packet_access_room_deny* pck = reinterpret_cast<SC::packet_access_room_deny*>(packet);
		// show room access deny message

	#ifdef START_GAME_INSTANT
		mNetPtr->Client()->RequestNewRoom();
	#endif
		break;
	}
	case SC::ROOM_INSIDE_INFO:
	{
		OutputDebugString(L"Received room inside info packet.\n");
		SC::packet_room_inside_info* pck = reinterpret_cast<SC::packet_room_inside_info*>(packet);
		mNetPtr->InitRoomInfo(pck);
		break;
	}
	case SC::ROOM_OUTSIDE_INFO:
	{
		OutputDebugString(L"Received room outside info packet.\n");
		
		SC::packet_room_outside_info* pck = reinterpret_cast<SC::packet_room_outside_info*>(packet);
		// TODO: need to lock container
		if(pck->room_closed == false) 
		{
			mRoomList[pck->room_id] = Room{ 
					  pck->room_id, 
					  pck->player_count, 
					  pck->map_id, 
					  pck->game_started, 
					  pck->room_closed };
		}
		else
		{
			mRoomList.erase(pck->room_id);
		}
	#ifdef START_GAME_INSTANT
		mNetPtr->Client()->RequestEnterRoom(0);
	#endif
		break;
	}
	case SC::FORCE_LOGOUT:
	{
		OutputDebugString(L"Received force logout packet.\n");
		SetSceneChangeFlag(SCENE_CHANGE_FLAG::LOGOUT);
		break;
	}
	default:
		OutputDebugString(L"Invalid packet.\n");
		return false;
	}
	return true;
}