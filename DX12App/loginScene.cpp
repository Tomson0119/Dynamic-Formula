#include "stdafx.h"

#include "NetLib/NetModule.h"
#include "LoginUI.h"
#pragma once
#include "loginScene.h"

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

void LoginScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, float Width, float Height, float aspect,
	std::shared_ptr<btDiscreteDynamicsWorld>& dynamicsWorld)
void LoginScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<BulletWrapper> physics)
{
	mDevice = device;
	mpUI = std::make_unique<LoginUI>(nFrame, device, cmdQueue);
	mpUI.get()->PreDraw(backBuffer, Width, Height);
}

void LoginScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<BulletWrapper> physics)
{
	
}

void LoginScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, UINT nFrame)
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
