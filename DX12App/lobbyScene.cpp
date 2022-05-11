#include "stdafx.h"
#include "lobbyScene.h"
#include "NetLib/NetModule.h"

LobbyScene::LobbyScene(HWND hwnd, NetModule* netPtr)
	: Scene{ hwnd, SCENE_STAT::LOBBY, (XMFLOAT4)Colors::White, netPtr }
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
}

void LobbyScene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
	float dx = static_cast<float>(x);
	float dy = static_cast<float>(y);
	mpUI->OnProcessMouseMove(btnState, x, y);
}

void LobbyScene::OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y)
{
	if (mpUI->OnProcessMouseClick(buttonState, x, y) == 1) // Click MakeRoom
	{

	}
	else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 2) // Click Room1
	{

	}
	else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 3) // Click Room2
	{

	}
	else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 4) // Click Room3
	{

	}
	else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 5) // Click Room4
	{

	}
	else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 6) // Click Room5
	{

	}
	else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 7) // Click Room6
	{

	}
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
			break;

		case static_cast<char>(ROOM_STAT::MAX_ROOM_REACHED):
			break;

		case static_cast<char>(ROOM_STAT::ROOM_IS_CLOSED):
			mNetPtr->Client()->RequestNewRoom(); // TEST
			break;

		case static_cast<char>(ROOM_STAT::ROOM_IS_FULL):
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
		// TODO: need to lock container
		if(pck->room_closed == false) 
		{
			mRoomList[pck->room_id] = Room{ 
					  pck->room_id, 
					  pck->player_count, 
					  pck->map_id, 
					  pck->game_started, 
					  pck->room_closed };

			// 规 积己
			// 
			//mpUI->setRoomActive(pck->room_id);
		}
		else
		{
			mRoomList.erase(pck->room_id);

			// 规 力芭
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