#include "stdafx.h"
#include "loginScene.h"
#include "NetLib/NetModule.h"


LoginScene::LoginScene(HWND hwnd, NetModule* netPtr)
	: Scene{ hwnd, SCENE_STAT::LOGIN, (XMFLOAT4)Colors::White, netPtr }
{
	OutputDebugStringW(L"Login Scene Entered.\n");
	Texts.resize(2);
}

void LoginScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	const std::shared_ptr<BulletWrapper>& physics)
{	
	mDevice = device;
	mpUI = std::make_unique<LoginUI>(nFrame, mDevice, cmdQueue);
	SetSound();
	mpUI->SetSound(GetSound());
	mpUI->BuildObjects(backBuffer, static_cast<UINT>(Width), static_cast<UINT>(Height));
}

void LoginScene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
	float dx = static_cast<float>(x);
	float dy = static_cast<float>(y);
	mpUI->OnProcessMouseMove(btnState, x, y);
}

void LoginScene::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
	if (buttonState&MK_LBUTTON) 
	{
		if (mpUI->OnProcessMouseClick(buttonState, x, y) == 1) // Login Button
		{
			OutputDebugStringA("Login button");
		#ifndef STADNALONE
			mID.assign(Texts[0]);
			mPWD.assign(Texts[1]);
			mNetPtr->Client()->RequestLogin(mID, mPWD);
		#endif
		}
		else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 2) // Sign-up Button
		{
		#ifndef STADNALONE
			mID.assign(Texts[0]);
			mPWD.assign(Texts[1]);
			mNetPtr->Client()->RequestRegister(mID, mPWD);
		#endif
		}
		else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 3) // Exit Button
		{
			//exit(0); //Error Occur
		}
		else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 4) // ID Button
		{
			IsPwd = false;
		}
		else if (mpUI->OnProcessMouseClick(buttonState, x, y) == 5) // PWD Button
		{
			IsPwd = true;
		}
	}
}

void LoginScene::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_KEYUP:
		if (wParam == VK_HOME)
		{
		#ifdef STANDALONE
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		#else
			mNetPtr->Client()->RequestLogin("GM", "GM");
		#endif
		}
		else if (wParam == VK_SHIFT) // Shift
		{
			IsShift = false;
		}
		break;

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
			//Login Check -> mID?? mPWD?? Login Check
			mNetPtr->Client()->RequestLogin(Texts[0], Texts[1]);
			break;
		case 0x08:  // backspace
			if(!Texts[IsPwd].empty() && (Texts[0].compare("ID") != 0||Texts[1].compare("Password") != 0))
				Texts[IsPwd].pop_back();
			break;
		case VK_SHIFT:
			IsShift = true;
			break;
		}
		
		if (!( (wParam < 58 && wParam>47) || (wParam > 64 && wParam < 91) || (wParam > 96 && wParam < 123) || wParam == 32) || 
			Texts[IsPwd].size() > 12 )   //???? or ?????? ???? ?????? ????, ?????????? ??????(32??)
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
			if (IsShift)
				Texts[IsPwd].push_back(static_cast<char>(wParam));
			else
				Texts[IsPwd].push_back(tolower(static_cast<int>(wParam)));
		}
		else
		{
			if (IsShift)
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
		mpUI->ChangeTextAlignment(0, 1/*Center*/); // 1== Center(CenterAllignment), 0 == Leading(LeftSideAllignment)
	}
	if (Texts[1].empty())
	{
		Texts[1].assign("Password");
		mpUI->ChangeTextAlignment(1, 1/*Center*/); 
	}

	std::vector<std::string> vTexts;

	vTexts.resize(Texts.size());
	for(int i=0;i<Texts.size();++i)
		vTexts[i].assign(Texts[i].begin(), Texts[i].end());

	auto& sound = GetSound();
	sound.Update();
	mpUI->SetIdPwdUIChecked(IsPwd);
	mpUI->Update(timer.TotalTime(), vTexts);
}

void LoginScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame)
{
	mpUI->Draw(nFrame);
}

bool LoginScene::ProcessPacket(std::byte* packet, const SC::PCK_TYPE& type, int bytes)
{
	auto& sound = GetSound();
	switch (type)
	{
	case SC::PCK_TYPE::LOGIN_RESULT:
	{
		OutputDebugString(L"Received login result packet.\n");
		
		SC::packet_login_result* pck = reinterpret_cast<SC::packet_login_result*>(packet);
		if (pck->result == static_cast<char>(LOGIN_STAT::ACCEPTED))
		{
			sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_IngameUI_SOUND_TRACK::LOGIN));
			//mNetPtr->Client()->BindUDPSocket(pck->port);
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::PUSH);
		}
		else if (pck->result == static_cast<char>(LOGIN_STAT::INVALID_IDPWD))
		{
			// INVALID ID OR PASSWOD;
			sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_IngameUI_SOUND_TRACK::ERR));
			mpUI->SetFailMessage(3); // 3 == Invalid ID or Password
			mpUI->SetFailBox(1);
		}
		break;
	}
	case SC::PCK_TYPE::REGISTER_RESULT:
	{
		OutputDebugStringW(L"Received register result packet.\n");
		
		SC::packet_register_result* pck = reinterpret_cast<SC::packet_register_result*>(packet);
		if(pck->result == static_cast<char>(REGI_STAT::ACCEPTED))
		{
			//
			sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_IngameUI_SOUND_TRACK::SIGNUP));
			mpUI->SetFailMessage(2);
			mpUI->SetFailBox(1);
		}
		else if (pck->result == static_cast<char>(REGI_STAT::ALREADY_EXIST))
		{
			//
			sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_IngameUI_SOUND_TRACK::ERR));
			mpUI->SetFailMessage(1);
			mpUI->SetFailBox(1);
		}
		else if (pck->result == static_cast<char>(REGI_STAT::INVALID_IDPWD))
		{
			//
			sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_IngameUI_SOUND_TRACK::ERR));
			mpUI->SetFailMessage(3);
			mpUI->SetFailBox(1);
		}
		break;
	}
	case SC::PCK_TYPE::ROOM_OUTSIDE_INFO:
	{
		OutputDebugString(L"Received room outside info packet.\n");
		SC::packet_room_outside_info* pck = reinterpret_cast<SC::packet_room_outside_info*>(packet);
		sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_IngameUI_SOUND_TRACK::GENERAL));

		mNetPtr->UpdateRoomList(pck);
		mNetPtr->SetIsUpdatedRoomList(true);
		break;
	}
	default:
		OutputDebugStringW(L"Invalid packet.\n");
		return false;
	}
	return true;
}

void LoginScene::Reset()
{
	mID.clear();
	mPWD.clear();
	Texts.clear();
	IsPwd = false;
}

void LoginScene::SetSound()
{
	std::vector<std::string> SoundFiles;
	SoundFiles.push_back("Sound/LoginSound/ErrorUI.wav");
	SoundFiles.push_back("Sound/LoginSound/GeneralUI.wav");
	SoundFiles.push_back("Sound/LoginSound/LoginUI.wav");
	SoundFiles.push_back("Sound/LoginSound/MouseCollisionCheckUI.wav");
	SoundFiles.push_back("Sound/LoginSound/SignUpUI.wav");


	std::vector<FMOD_MODE> modes;
	modes.push_back(FMOD_LOOP_OFF);
	modes.push_back(FMOD_LOOP_OFF);
	modes.push_back(FMOD_LOOP_OFF);
	modes.push_back(FMOD_LOOP_OFF);
	modes.push_back(FMOD_LOOP_OFF);


	GetSound().InitSound(SoundFiles, modes);
}
