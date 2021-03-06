#pragma once

#include "gameTimer.h"
#include "mesh.h"
#include "gameObject.h"
#include "sound.h"

class NetModule;
class UI;

enum class SCENE_STAT : char
{
	NONE = 0,
	LOGIN,
	LOBBY,
	ROOM,
	IN_GAME
};

enum class SCENE_CHANGE_FLAG : char
{
	NONE=0,
	PUSH,
	POP,
	LOGOUT
};

class Scene
{
public:
	Scene(HWND hwnd, SCENE_STAT stat, const XMFLOAT4& color, NetModule* netPtr);
	Scene(const Scene& rhs) = delete;
	Scene& operator=(const Scene& rhs) = delete;
	virtual ~Scene() = default;

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
		const std::shared_ptr<BulletWrapper>& physics) = 0;

	virtual void Update(
		ID3D12GraphicsCommandList* cmdList, 
		const GameTimer& timer,
		const std::shared_ptr<BulletWrapper>& physics) = 0;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame) = 0;
	virtual bool ProcessPacket(std::byte* packet, const SC::PCK_TYPE& type, int bytes) = 0;
	virtual UI* GetUI() const = 0;

	virtual void Reset() = 0;

	virtual void OnResize(float aspect) { }
	virtual void ProcessAfterResize() {}
	virtual void PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed) { }

	virtual void OnProcessMouseDown(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseUp(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y);
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual ID3D12RootSignature* GetRootSignature() const { return nullptr; }
	virtual void SetSound() = 0;
public:
	SCENE_CHANGE_FLAG GetSceneChangeFlag() const { return mSceneChangeFlag; }
	void SetSceneChangeFlag(SCENE_CHANGE_FLAG flag) { mSceneChangeFlag = flag; }
	SCENE_STAT GetSceneState() const { return mSceneState; }
	const XMFLOAT4& GetFrameColor() { return mFrameColor; }
	Sound& GetSound() { return mSound; }

protected:
	SCENE_STAT mSceneState;
	XMFLOAT4 mFrameColor;

	NetModule* mNetPtr;

	std::atomic<SCENE_CHANGE_FLAG> mSceneChangeFlag;
	ComPtr<ID3D12Device> mDevice;

	HWND mHwnd;

private:
	Sound mSound;
};