#pragma once

#include "scene.h"

class NetModule;

class RoomScene : public Scene
{
public:
	RoomScene(HWND hwnd, NetModule* netPtr);
	virtual ~RoomScene() = default;

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
	std::chrono::high_resolution_clock::time_point mStartTime;
	std::atomic_bool mSendFlag;
};