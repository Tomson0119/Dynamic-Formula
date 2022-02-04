#pragma once

#include "scene.h"

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
class NetModule;

class InGameScene : public Scene
{
public:
	InGameScene(NetModule* netPtr);
	virtual ~InGameScene();

public:
	virtual void OnResize(float aspect) override;

	virtual void BuildObjects(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		float aspect,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicsWorld) override;
	
	virtual void Update(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList, 
		const GameTimer& timer,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicsWorld) override;
	
	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer) override;
	virtual void PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed) override;

	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;

public:
	void UpdateLight(float elapsed);
	void UpdateLightConstants();
	void UpdateCameraConstant(int idx, Camera* camera);
	void UpdateConstants(const GameTimer& timer);	

	void SetCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex = 0);

	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex=0);
	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, Camera* camera, int cameraCBIndex = 0);

	void OnPreciseKeyInput(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld, float elapsed);
	
public:
	virtual void OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseUp(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual ID3D12RootSignature* GetRootSignature() const override { return mRootSignature.Get(); }
	virtual Player* GetPlayer() { return mPlayer; }
private:
	void BuildRootSignature(ID3D12Device* device);
	void BuildComputeRootSignature(ID3D12Device* device);
	void BuildGameObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld>& dynamicsWorld);
	void BuildConstantBuffers(ID3D12Device* device);
	void BuildShadersAndPSOs(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void BuildDescriptorHeap(ID3D12Device* device);

	void CreateVelocityMapViews(ID3D12Device* device);
	void CreateVelocityMapDescriptorHeaps(ID3D12Device* device);

	void AppendMissileObject(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);
	void UpdateMissileObject(ID3D12Device* device, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);

private:
	std::unique_ptr<Camera> mMainCamera;
	POINT mLastMousePos{};

	float mCameraRadius = 30.0f;

	LightConstants mMainLight;

	D3D12_CPU_DESCRIPTOR_HANDLE mVelocityMapRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE mVelocityMapSrvHandle;
	ComPtr<ID3D12Resource> mVelocityMap;

	ComPtr<ID3D12DescriptorHeap> mVelocityMapRtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mVelocityMapSrvDescriptorHeap;

	std::unique_ptr<ConstantBuffer<CameraConstants>> mCameraCB;
	std::unique_ptr<ConstantBuffer<LightConstants>> mLightCB;
	std::unique_ptr<ConstantBuffer<GameInfoConstants>> mGameInfoCB;

	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12RootSignature> mComputeRootSignature;

	std::map<Layer, std::unique_ptr<Pipeline>> mPipelines;
	std::map<Layer, std::unique_ptr<ComputePipeline>> mPostProcessingPipelines;
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
	float mCubemapInterval = 0.0f;

	UINT mCubemapDrawIndex = 0;
};