#pragma once

#include "scene.h"

class NetModule;

class LoginScene : public Scene
{
public:
	LoginScene(NetModule* netPtr);
	virtual ~LoginScene() = default;

public:
	virtual void BuildObjects(
		ComPtr<ID3D12Device> device,
		ID3D12GraphicsCommandList* cmdList,
		float aspect,
		std::shared_ptr<BulletWrapper> physics) override;

	virtual void Update(
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		std::shared_ptr<BulletWrapper> physics) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer) override;

	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;

private:

};