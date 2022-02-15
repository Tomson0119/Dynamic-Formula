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
class PhysicsPlayer;

class InGameScene : public Scene
{
public:
	InGameScene(NetModule* netPtr);
	virtual ~InGameScene();

public:
	virtual void OnResize(float aspect) override;

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
	virtual void PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed) override;

	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;

public:
	void UpdateLight(float elapsed);
	void UpdateLightConstants();
	void UpdateCameraConstant(int idx, Camera* camera);
	void UpdateConstants(const GameTimer& timer);
	//void UpdateDynamicsWorld();

	void SetCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex = 0);

	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex=0);
	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, Camera* camera, int cameraCBIndex = 0);

	void OnPreciseKeyInput(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<BulletWrapper> physics, float elapsed);

public:
	virtual void OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseUp(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual ID3D12RootSignature* GetRootSignature() const override { return mRootSignature.Get(); }

private:
	void BuildRootSignature();
	void BuildComputeRootSignature();
	void BuildGameObjects(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<BulletWrapper>& physics);
	void BuildConstantBuffers();
	void BuildShadersAndPSOs(ID3D12GraphicsCommandList* cmdList);
	void BuildDescriptorHeap();

	void BuildCarObjects(
		const XMFLOAT3& position,
		char color,
		bool isPlayer,
		ID3D12GraphicsCommandList* cmdList, 
		std::shared_ptr<BulletWrapper>& dynamicsWorld, UINT netID);

	void CreateVelocityMapViews();
	void CreateVelocityMapDescriptorHeaps();

	void AppendMissileObject(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<BulletWrapper> physics);
	void UpdateMissileObject();

	void UpdatePlayerObjects();

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

	std::map<MeshType, std::vector<std::shared_ptr<Mesh>>> mMeshList;

	std::unique_ptr<ShadowMapRenderer> mShadowMapRenderer;

	Player* mPlayer = nullptr;
	std::vector<std::shared_ptr<MissileObject>> mMissileObjects;
	std::vector<std::shared_ptr<PhysicsPlayer>> mPlayerObjects;

	std::shared_ptr<Billboard> mFlameBillboard;
	std::shared_ptr<Billboard> mDustBillboard;
	std::shared_ptr<btDiscreteDynamicsWorld> mDynamicsWorld;

	std::chrono::high_resolution_clock::time_point mPrevTime;

	const XMFLOAT3 mRoomCenter = { -1024, 0, 1024 };

	bool mLODSet = false;
	bool mOutside = false;

	float mMissileInterval = 0.0f;
	float mCubemapInterval = 0.0f;

	UINT mCubemapDrawIndex = 0;

	// Key pressed flag
	std::map<int, bool> mKeyMap;

	// color map
	const std::array<XMFLOAT4, MAX_ROOM_CAPACITY> mColorMap = {
		(XMFLOAT4)Colors::Red, (XMFLOAT4)Colors::Blue,
		(XMFLOAT4)Colors::Green, (XMFLOAT4)Colors::Purple,
		(XMFLOAT4)Colors::Black, (XMFLOAT4)Colors::White,
		(XMFLOAT4)Colors::Orange, (XMFLOAT4)Colors::Yellow
	};
};