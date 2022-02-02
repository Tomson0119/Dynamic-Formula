#include "stdafx.h"
#include "roomScene.h"
#include "NetLib/NetModule.h"

RoomScene::RoomScene(NetModule* netPtr)
	: Scene{ SCENE_STAT::ROOM, (XMFLOAT4)Colors::Chocolate, netPtr }
{
	OutputDebugStringW(L"Room Scene Entered.\n");
	SetSceneFlag(true);
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
