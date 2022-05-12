#include "stdafx.h"
#include "lobbyScene.h"
#include "NetLib/NetModule.h"

LobbyScene::LobbyScene(HWND hwnd, NetModule* netPtr)
	: Scene{ hwnd, SCENE_STAT::LOBBY, (XMFLOAT4)Colors::White, netPtr },
	  mPageNum{ 0 }
{
	OutputDebugStringW(L"Lobby Scene Entered.\n");
}

void LobbyScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	const std::shared_ptr<BulletWrapper>& physics)
{
	mDevice = device;
	mpUI = std::make_unique<LobbyUI>(nFrame, mDevice, cmdQueue);
	mpUI->BuildObjects(backBuffer, static_cast<UINT>(Width), static_cast<UINT>(Height));
}

void LobbyScene::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_HOME:
		{
		#ifdef STANDALONE
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		#else
			mNetPtr->Client()->RequestEnterRoom(0); // TEST
		#endif
			break;
		}
		case VK_END:
		{
			mNetPtr->Client()->RevertScene();
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::POP);
			break;
		}
		}
	}
	mpUI->OnProcessKeyInput(msg, wParam, lParam);
}

void LobbyScene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
	float dx = static_cast<float>(x);
	float dy = static_cast<float>(y);
	mpUI->OnProcessMouseMove(btnState, x, y);
}

void LobbyScene::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
	if (buttonState & MK_LBUTTON)
	{
		int ret = mpUI->OnProcessMouseClick(buttonState, x, y);
		if (ret == 0) // Click MakeRoom
		{
			OutputDebugStringA("MakeRoom Button Down\n");
#ifndef STANDALONE
			mNetPtr->Client()->RequestNewRoom();
#endif
		}
		else // return RoomNum
		{
			OutputDebugStringA("Room Enter Button Down");
			OutputDebugStringA(std::to_string(ret).c_str());
			OutputDebugStringA("\n");
#ifndef STANDALONE
			mNetPtr->Client()->RequestEnterRoom(ret);
#endif
		}
	}
	mpUI->OnProcessMouseDown(buttonState, x, y);
}

void LobbyScene::OnProcessMouseUp(WPARAM buttonState, int x, int y)
{

}

void LobbyScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, const std::shared_ptr<BulletWrapper>& physics)
{
	mpUI->Update(timer.TotalTime());
}

void LobbyScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame)
{
	mpUI->Draw(nFrame);
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
		switch (pck->reason)
		{
		case static_cast<char>(ROOM_STAT::GAME_STARTED):
			mpUI->UpdateDenyBoxText("Already Started");
			mpUI->SetDenyBox();
			break;

		case static_cast<char>(ROOM_STAT::MAX_ROOM_REACHED):
			mpUI->UpdateDenyBoxText("Max Room State");
			mpUI->SetDenyBox();
			break;

		case static_cast<char>(ROOM_STAT::ROOM_IS_CLOSED):
			mNetPtr->Client()->RequestNewRoom(); // TEST
			break;

		case static_cast<char>(ROOM_STAT::ROOM_IS_FULL):
			mpUI->UpdateDenyBoxText("Room Is Full");
			mpUI->SetDenyBox();
			break;
		}
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
		if(pck->room_closed == false) 
		{
			mRoomListMut.lock();
			mRoomList[pck->room_id] = Room{ 
					  pck->room_id, 
					  pck->player_count, 
					  pck->map_id, 
					  pck->game_started };
			mRoomListMut.unlock();
		}
		else
		{
			mRoomListMut.lock();
			mRoomList.erase(pck->room_id);
			mRoomListMut.unlock();
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