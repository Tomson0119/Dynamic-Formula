#include "stdafx.h"
#include "loginScene.h"

LoginScene::LoginScene()
	: Scene(SCENE_STAT::LOGIN)
{
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