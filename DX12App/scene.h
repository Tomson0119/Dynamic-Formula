#pragma once

#include "gameTimer.h"

enum class SCENE_STAT : char
{
	NONE = 0,
	LOGIN,
	LOBBY,
	ROOM,
	IN_GAME
};

class NetModule;

class Scene
{
public:
	explicit Scene(SCENE_STAT stat, const XMFLOAT4& color, NetModule* netPtr);
	Scene(const Scene& rhs) = delete;
	Scene& operator=(const Scene& rhs) = delete;
	virtual ~Scene() = default;

public:
	virtual void BuildObjects(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList, 
		float aspect,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld) = 0;

	virtual void Update(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList, 
		const GameTimer& timer,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld) = 0;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer) = 0;
	
	virtual void OnResize(float aspect) { }
	virtual void PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed) { }

	virtual void OnProcessMouseDown(HWND hwnd, WPARAM btnState, int x, int y);
	virtual void OnProcessMouseUp(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y);
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual ID3D12RootSignature* GetRootSignature() const { return nullptr; }

public:
	void SetSceneFlag(bool flag) { mSceneChangeFlag = flag; }

	bool NeedToChangeScene() const { return mSceneChangeFlag; }
	SCENE_STAT GetSceneState() const { return mSceneState; }
	const XMFLOAT4& GetFrameColor() { return mFrameColor; }

protected:
	SCENE_STAT mSceneState;
	XMFLOAT4 mFrameColor;

	NetModule* mNetPtr;

	std::atomic_bool mSceneChangeFlag;
};