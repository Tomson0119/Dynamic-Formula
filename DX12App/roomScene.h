#pragma once

#include "scene.h"
#include "RoomUI.h"

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
		ID3D12CommandQueue* cmdQueue,
		UINT nFrame,
		ID3D12Resource** backBuffer,
		float Width,
		float Height,
		float aspect,
		const std::shared_ptr<BulletWrapper>& physics) override;

	virtual void Update(
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		const std::shared_ptr<BulletWrapper>& physics) override;
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseDown(WPARAM btnState, int x, int y) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame) override;
	
	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;
	virtual UI* GetUI() const override { return mpUI.get(); }

private:
	std::chrono::high_resolution_clock::time_point mStartTime;
	std::atomic_bool mSendFlag;
	std::unique_ptr<RoomUI> mpUI;
};