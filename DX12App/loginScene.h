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
		ID3D12CommandQueue* cmdQueue,
		UINT nFrame,
		ID3D12Resource** backBuffer,
		float Width,
		float Height,
		float aspect,
		std::shared_ptr<BulletWrapper> physics) override;

	virtual void Update(
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		std::shared_ptr<BulletWrapper> physics) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, UINT nFrame) override;
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y);

	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;
	void KeyInputFunc();
	//std::string GetId() { return id; }
private:
	std::string mID;
	std::string mPWD;
	std::vector<std::string> Texts;
	bool IsPwd = false;
};