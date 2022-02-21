#
#include "stdafx.h"

#include "LobbyUI.h"
#include "NetLib/NetModule.h"
#pragma once
#include "lobbyScene.h"
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

void LobbyScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	std::shared_ptr<BulletWrapper> physics)
{
	mDevice = device;
	mpUI = std::make_unique<LobbyUI>(nFrame, mDevice, cmdQueue);
	mpUI.get()->PreDraw(backBuffer, Width, Height);
}

void LobbyScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<BulletWrapper> physics)
{
	mpUI.get()->Update(timer.TotalTime());
}

void LobbyScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, UINT nFrame)
{
	mpUI.get()->Draw(nFrame);
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