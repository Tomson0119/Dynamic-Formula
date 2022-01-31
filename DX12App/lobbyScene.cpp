#include "stdafx.h"
#include "lobbyScene.h"

LobbyScene::LobbyScene()
	: Scene(SCENE_STAT::LOBBY)
{
}

void LobbyScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void LobbyScene::Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void LobbyScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView)
{
}
