#include "stdafx.h"
#include "loginScene.h"
#include "NetLib/NetModule.h"
#include "LoginUI.h"


LoginScene::LoginScene(HWND hwnd, NetModule* netPtr)
	: Scene{ hwnd, SCENE_STAT::LOGIN, (XMFLOAT4)Colors::Aqua, netPtr }
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
	SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
#endif
}

void LoginScene::KeyInputFunc()
{
}

void LoginScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	const std::shared_ptr<BulletWrapper>& physics)
{
	mDevice = device;
	mpUI = std::make_unique<LoginUI>(nFrame, mDevice, cmdQueue);
	mpUI.get()->PreDraw(backBuffer, static_cast<UINT>(Width), static_cast<UINT>(Height));
}

void LoginScene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
	float dx = static_cast<float>(x);
	float dy = static_cast<float>(y);
	mpUI.get()->OnProcessMouseMove(btnState, x, y);
}

void LoginScene::OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y)
{
	if (buttonState) 
	{
		//LoginCheck
		if (mpUI.get()->OnProcessMouseDown(hwnd, buttonState, x, y))
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
	}
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
			mID.assign(Texts[0].begin(), Texts[0].end());
			mPWD.assign(Texts[1].begin(), Texts[1].end());
			//Login Check -> mID와 mPWD로 Login Check
			break;
		case 0x08:  // backspace
			if(!Texts[IsPwd].empty() && (Texts[0].compare("ID") != 0||Texts[1].compare("Password") != 0))
				Texts[IsPwd].pop_back();
			break;
		case VK_HOME:
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
			return;
		case VK_END:
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::POP);
			return;
		}
		
		if (!( (wParam < 57 && wParam>47) || (wParam > 64 && wParam < 91) || (wParam > 96 && wParam < 123) || wParam == 32) || 
			Texts[IsPwd].size() > 12 )   //숫자 or 문자가 아닌 경우는 제외, 스페이스는 검사함(32번)
			break;
		if (!IsPwd && Texts[IsPwd] == "ID")
		{
			Texts[IsPwd].clear();
			mpUI->ChangeTextAlignment(IsPwd, 0);
		}
		if (IsPwd && Texts[IsPwd] == "Password")
		{
			Texts[IsPwd].clear();
			mpUI->ChangeTextAlignment(IsPwd, 0);
		}
		if ((GetKeyState(VK_CAPITAL) & 0x0001) == 0)
		{
			if ((GetAsyncKeyState(VK_SHIFT) & 0x0001) == 1)
				Texts[IsPwd].push_back(static_cast<char>(wParam));
			else
				Texts[IsPwd].push_back(tolower(static_cast<int>(wParam)));
		}
		else
		{
			if ((GetAsyncKeyState(VK_SHIFT) & 0x0001) == 1)
				Texts[IsPwd].push_back(tolower(static_cast<int>(wParam)));
			else
				Texts[IsPwd].push_back(static_cast<char>(wParam));
		}
	}
}

void LoginScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, const std::shared_ptr<BulletWrapper>& physics)
{
	if (Texts[0].empty())
	{
		Texts[0].assign("ID");
		mpUI->ChangeTextAlignment(0, 1);
	}
	if (Texts[1].empty())
	{
		Texts[1].assign("Password");
		mpUI->ChangeTextAlignment(1, 1);
	}

	std::vector<std::wstring> WTexts;

	WTexts.resize(Texts.size());
	for(int i=0;i<Texts.size();++i)
		WTexts[i].assign(Texts[i].begin(), Texts[i].end());

	mpUI.get()->Update(timer.TotalTime(), WTexts);
}

void LoginScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame)
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
		{
			mNetPtr->Client()->BindUDPSocket(pck->port);
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		}
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
