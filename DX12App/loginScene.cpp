#include "stdafx.h"
#include "loginScene.h"
#include "NetLib/NetModule.h"

//#define STANDALONE

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
	mSceneChangeFlag = true;
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