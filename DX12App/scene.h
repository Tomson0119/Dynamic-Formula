#pragma once

#include "gameTimer.h"
#include "mesh.h"
#include "gameObject.h"

//#define STANDALONE
#define START_GAME_INSTANT

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

class NetModule;

class Scene
{
public:
	explicit Scene(HWND hwnd, SCENE_STAT stat, const XMFLOAT4& color, NetModule* netPtr);
	Scene(const Scene& rhs) = delete;
	Scene& operator=(const Scene& rhs) = delete;
	virtual ~Scene() = default;

public:
	virtual void BuildObjects(
		ComPtr<ID3D12Device> device,
		ID3D12GraphicsCommandList* cmdList, 
		float aspect,
		std::shared_ptr<BulletWrapper> physics) = 0;

	virtual void Update(
		ID3D12GraphicsCommandList* cmdList, 
		const GameTimer& timer,
		std::shared_ptr<BulletWrapper> physics) = 0;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer) = 0;
	
	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) = 0;

	virtual void OnResize(float aspect) { }
	virtual void PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed) { }

	virtual void OnProcessMouseDown(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseUp(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y);
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual ID3D12RootSignature* GetRootSignature() const { return nullptr; }

public:
	SCENE_CHANGE_FLAG GetSceneChangeFlag() const { return mSceneChangeFlag; }
	void SetSceneChangeFlag(SCENE_CHANGE_FLAG flag) { mSceneChangeFlag = flag; }

	SCENE_STAT GetSceneState() const { return mSceneState; }
	const XMFLOAT4& GetFrameColor() { return mFrameColor; }

protected:
	SCENE_STAT mSceneState;
	XMFLOAT4 mFrameColor;

	NetModule* mNetPtr;

	std::atomic<SCENE_CHANGE_FLAG> mSceneChangeFlag;
	ComPtr<ID3D12Device> mDevice;

	HWND mHwnd;
};