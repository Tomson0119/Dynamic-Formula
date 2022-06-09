#include "stdafx.h"
#include "roomScene.h"
#include "NetLib/NetModule.h"

RoomScene::RoomScene(HWND hwnd, NetModule* netPtr)
	: Scene{ hwnd, SCENE_STAT::ROOM, (XMFLOAT4)Colors::Black, netPtr }
{
	OutputDebugStringW(L"Room Scene Entered.\n");
}

void RoomScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	const std::shared_ptr<BulletWrapper>& physics)
{
	mDevice = device;
	mpUI = std::make_unique<RoomUI>(nFrame, mDevice, cmdQueue, *mNetPtr);
	mpUI->BuildObjects(backBuffer, static_cast<UINT>(Width), static_cast<UINT>(Height));
}

void RoomScene::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
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
			int roomId = mNetPtr->GetRoomID();
			mNetPtr->Client()->ToggleReady(roomId);
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

void RoomScene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
	mpUI->OnProcessMouseMove(btnState, x, y);
}

void RoomScene::OnProcessMouseDown(WPARAM btnState, int x, int y)
{
	mpUI->OnProcessMouseDown(btnState, x, y);
}

void RoomScene::OnProcessMouseUp(WPARAM btnState, int x, int y)
{
	// 맵 변경 버튼
	if (mpUI->OnProcessMouseClick(btnState, x, y) == -1) // 맵변경
	{

	}
	else if (mpUI->OnProcessMouseClick(btnState, x, y) == -2) // 나가기
	{
		mNetPtr->Client()->RevertScene();
		SetSceneChangeFlag(SCENE_CHANGE_FLAG::POP);
	}
	else if (mpUI->OnProcessMouseClick(btnState, x, y) == 0) // 예외
		return;
	else if (mpUI->OnProcessMouseClick(btnState, x, y) == 1)// Ready
	{
		mNetPtr->Client()->ToggleReady(mNetPtr->GetRoomID());
	}
}

void RoomScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, const std::shared_ptr<BulletWrapper>& physics)
{
	mpUI->Update(timer.TotalTime());
	if (mpUI->GetLodingUpdated())
		SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
}

void RoomScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame)
{
	mpUI->Draw(nFrame);
}

bool RoomScene::ProcessPacket(std::byte* packet, const SC::PCK_TYPE& type, int bytes)
{
	switch (type)
	{
	case SC::PCK_TYPE::ROOM_INSIDE_INFO:
	{
		OutputDebugString(L"Received room inside info packet.\n");
		SC::packet_room_inside_info* pck = reinterpret_cast<SC::packet_room_inside_info*>(packet);
		mNetPtr->InitRoomInfo(pck);		
		break;
	}
	case SC::PCK_TYPE::ROOM_OUTSIDE_INFO:
	{
		OutputDebugString(L"Received room outside info packet.\n");
		SC::packet_room_outside_info* pck = reinterpret_cast<SC::packet_room_outside_info*>(packet);
		mNetPtr->UpdateRoomList(pck);
		mNetPtr->SetIsUpdatedRoomList(true);
		break;
	}
	case SC::PCK_TYPE::UPDATE_PLAYER_INFO:
	{
		SC::packet_update_player_info* pck = reinterpret_cast<SC::packet_update_player_info*>(packet);
		mNetPtr->UpdatePlayerInfo(pck);
		break;
	}
	case SC::PCK_TYPE::UPDATE_MAP_INFO:
	{
		OutputDebugString(L"Received update map info packet.\n");
		SC::packet_update_map_info* pck = reinterpret_cast<SC::packet_update_map_info*>(packet);
		mNetPtr->UpdateMapIndex(pck);
		//mpUI->SetMapID(static_cast<int>(pck->map_id));
		break;
	}
	case SC::PCK_TYPE::REMOVE_PLAYER:
	{
		OutputDebugString(L"Received remove player packet.\n");

		SC::packet_remove_player* pck = reinterpret_cast<SC::packet_remove_player*>(packet);
		mNetPtr->RemovePlayer(pck);
		break;
	}
	case SC::PCK_TYPE::GAME_START_SUCCESS:
	{
		OutputDebugString(L"Received game start packet.\n");

		SC::packet_game_start_success* pck = reinterpret_cast<SC::packet_game_start_success*>(packet);		
		mNetPtr->InitPlayerTransform(pck);


		//Scene Delete and TextOut
		mpUI->SetLodingScene();

		
		//mpUI->SetMyReadyOff();
		break;
	}
	case SC::PCK_TYPE::GAME_START_FAIL:
	{		
		SC::packet_game_start_fail* pck = reinterpret_cast<SC::packet_game_start_fail*>(packet);
		OutputDebugStringA("Not everyone is ready.\n");
		//mpUI->SetStateFail(0); // please multithread error!!!!!!!!!
		break;
	}
	case SC::PCK_TYPE::FORCE_LOGOUT:
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