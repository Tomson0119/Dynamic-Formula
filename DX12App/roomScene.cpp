#include "stdafx.h"
#include "roomScene.h"

RoomScene::RoomScene()
	: Scene(SCENE_STAT::ROOM)
{
}

void RoomScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void RoomScene::Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld)
{
}

void RoomScene::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer)
{
}
