#pragma once

#include "gameTimer.h"
#include "camera.h"
#include "constantBuffer.h"

#include "mesh.h"
#include "pipeline.h"
#include "player.h"
#include "shader.h"
#include "texture.h"

class DynamicCubeRenderer;
class ShadowMapRenderer;

class GameScene
{
public:
	GameScene();
	GameScene(const GameScene& rhs) = delete;
	GameScene& operator=(const GameScene& rhs) = delete;
	virtual ~GameScene();

	void OnResize(float aspect);

	void BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);
	
	void UpdateLight(float elapsed);
	void UpdateLightConstants();
	void UpdateCameraConstant(int idx, Camera* camera);
	void UpdateConstants(const GameTimer& timer);
	
	void Update(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld, const GameTimer& timer);

	void SetCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex = 0);
	void Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer);
	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex=0);

	void PreRender(ID3D12GraphicsCommandList* cmdList);

	void OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y);
	void OnProcessMouseUp(WPARAM buttonState, int x, int y);
	void OnProcessMouseMove(WPARAM buttonState, int x, int y);
	void OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPreciseKeyInput(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld, float elapsed);

	XMFLOAT4 GetFrameColor() const { return mFrameColor; }
	ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

private:
	void BuildRootSignature(ID3D12Device* device);
	void BuildComputeRootSignature(ID3D12Device* device);
	void BuildGameObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicsWorld);
	void BuildConstantBuffers(ID3D12Device* device);
	void BuildShadersAndPSOs(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void BuildDescriptorHeap(ID3D12Device* device);

	void AppendMissileObject(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);
	void UpdateMissileObject(ID3D12Device* device, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);

private:
	XMFLOAT4 mFrameColor = (XMFLOAT4)Colors::LightSkyBlue;

	std::unique_ptr<Camera> mMainCamera;
	POINT mLastMousePos{};

	float mCameraRadius = 30.0f;

	LightConstants mMainLight;

	std::unique_ptr<ConstantBuffer<CameraConstants>> mCameraCB;
	std::unique_ptr<ConstantBuffer<LightConstants>> mLightCB;
	std::unique_ptr<ConstantBuffer<GameInfoConstants>> mGameInfoCB;

	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12RootSignature> mComputeRootSignature;

	std::map<Layer, std::unique_ptr<Pipeline>> mPipelines;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	
	std::unique_ptr<ShadowMapRenderer> mShadowMapRenderer;

	Player* mPlayer = nullptr;
	std::vector<std::shared_ptr<MissileObject>> mMissileObjects;

	std::shared_ptr<Billboard> mFlameBillboard;
	std::shared_ptr<Billboard> mDustBillboard;
	std::shared_ptr<Mesh> mMissileMesh;
	std::chrono::high_resolution_clock::time_point mPrevTime;

	const XMFLOAT3 mRoomCenter = { -1024, 0, 1024 };

	bool mLODSet = false;
	bool mOutside = false;

	float mMissileInterval = 0.0f;
};