#include "stdafx.h"
#include "lobbyScene.h"
#include "NetLib/NetModule.h"

LobbyScene::LobbyScene(NetModule* netPtr)
	: Scene{ SCENE_STAT::LOBBY, (XMFLOAT4)Colors::Bisque, netPtr }
{
	OutputDebugStringW(L"Lobby Scene Entered.\n");
	SetSceneFlag(true);
}

void LobbyScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void LobbyScene::Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void LobbyScene::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer)
{
}
