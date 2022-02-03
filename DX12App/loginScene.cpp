#include "stdafx.h"
#include "loginScene.h"
#include "NetLib/NetModule.h"


LoginScene::LoginScene(NetModule* netPtr)
	: Scene{ SCENE_STAT::LOGIN, (XMFLOAT4)Colors::Aqua, netPtr }
{
	OutputDebugStringW(L"Login Scene Entered.\n");
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

void LoginScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void LoginScene::Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
	
}

void LoginScene::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer)
{
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
