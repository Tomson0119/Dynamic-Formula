#include "stdafx.h"

#include "NetLib/NetModule.h"
#include "LoginUI.h"
#pragma once
#include "loginScene.h"

LoginScene::LoginScene(NetModule* netPtr)
	: Scene{ SCENE_STAT::LOGIN, (XMFLOAT4)Colors::Aqua, netPtr }
{
	OutputDebugStringW(L"Login Scene Entered.\n");
	Texts.resize(2);
#ifndef STANDALONE
	if (mNetPtr->Connect(SERVER_IP, SERVER_PORT))
	{
		// TEST
		mNetPtr->Client()->RequestLogin("GM", "GM");
	}
	else OutputDebugStringW(L"Failed to connect to server.\n");
#else
	//SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
#endif
}
void LoginScene::KeyInputFunc()
{
}
void LoginScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	std::shared_ptr<BulletWrapper> physics)
{
	mDevice = device;
	mpUI = std::make_unique<LoginUI>(nFrame, mDevice, cmdQueue);
	mpUI.get()->PreDraw(backBuffer, Width, Height);
	//std::thread t1(KeyInputFunc);
}
void LoginScene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
	float dx = static_cast<float>(x);
	float dy = static_cast<float>(y);
	mpUI.get()->OnProcessMouseMove(btnState, x, y);
}
void LoginScene::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x0A:  // linefeed 
		case 0x1B:  // escape
			break;
		case 0x09:  // tab
			if (!IsPwd)	IsPwd = true;
			else IsPwd = false;
			break;
		case 0x0D:  // carriage return
			//Login Check
			break;
		case 0x08:  // backspace
			Texts[IsPwd].pop_back();
			break;
		case VK_HOME:
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		}
		if (
			(wParam < 57 && wParam>47) ||
			(wParam > 64 && wParam < 91) ||
			(wParam > 96 && wParam < 123)||
			wParam ==32
			)
		{
			if ((GetKeyState(VK_CAPITAL) & 0x0001) == 0)
				Texts[IsPwd].push_back(tolower(wParam));
			else 
				Texts[IsPwd].push_back(wParam);

		}
		break;
	}
	//mpUI.get()->Update(Texts);

	//case WM_CHAR:
	//	//mpUI.get()->OnProcessKeyInput(msg, wParam, lParam);
	//	switch (wParam)
	//	{
	//	case 0x08:  // backspace 
	//	case 0x0A:  // linefeed 
	//	case 0x1B:  // escape 
	//	case 0x09:  // tab 
	//	case 0x0D:  // carriage return
	//		break;
	//	default:    // displayable character 

	//		//TCHAR ch = (TCHAR)wParam;
	//		//Texts[0].push_back(ch);
	//		break;
	//	}
	//	break;
}

void LoginScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<BulletWrapper> physics)
{
	//Texts[0].clear();
	//Texts[0].push_back('c');
	for (auto ch : id)
		Texts[0].push_back(ch);
	mpUI.get()->Update(timer.TotalTime(), Texts);
}

void LoginScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, UINT nFrame)
{
	mpUI.get()->Draw(nFrame);
}

bool LoginScene::ProcessPacket(std::byte* packet, char type, int bytes)
{
	switch (type)
	{
	case SC::LOGIN_RESULT:
	{
		OutputDebugString(L"Received login result packet.\n");
		
		SC::packet_login_result* pck = reinterpret_cast<SC::packet_login_result*>(packet);
		if (pck->result == static_cast<char>(LOGIN_STAT::ACCEPTED))
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		else
			// Show login has failed message
			;
		break;
	}
	case SC::REGISTER_RESULT:
	{
		OutputDebugStringW(L"Received register result packet.\n");
		
		SC::packet_register_result* pck = reinterpret_cast<SC::packet_register_result*>(packet);
		// Show register result
		break;
	}
	default:
		OutputDebugStringW(L"Invalid packet.\n");
		return false;
	}
	return true;
}
