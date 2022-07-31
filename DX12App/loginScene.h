#pragma once

#include "scene.h"
#include "LoginUI.h"

class NetModule;

class LoginScene : public Scene
{
public:
	LoginScene(HWND hwnd, NetModule* netPtr);
	virtual ~LoginScene() = default;

public:
	virtual void BuildObjects(
		ID3D12Device* device,
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

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame) override;
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y);

	virtual bool ProcessPacket(std::byte* packet, const SC::PCK_TYPE& type, int bytes) override;

	virtual void Reset() override;
	
	virtual UI* GetUI() const override { return mpUI.get(); }
	virtual void SetSound();

private:
	std::string mID;
	std::string mPWD;
	std::vector<std::string> Texts;
	bool IsPwd = false;
	bool IsShift = false;

	std::unique_ptr<LoginUI> mpUI;
};