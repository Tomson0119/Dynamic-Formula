#pragma once

#include "scene.h"

class LobbyScene : public Scene
{
public:
	LobbyScene();
	virtual ~LobbyScene() = default;

public:
	virtual void BuildObjects(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		float aspect,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld) override;

	virtual void Update(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView) override;

private:

};