#pragma once

#include "pipeline.h"
#include "player.h"

#include "scene.h"
#include "InGameUI.h"

class DynamicCubeRenderer;
class ShadowMapRenderer;
class NetModule;
class PhysicsPlayer;

class InGameScene : public Scene
{
public:
	InGameScene(HWND hwnd, NetModule* netPtr, bool msaaEnable, UINT msaaQuality);
	virtual ~InGameScene();

public:
	virtual void OnResize(float aspect) ;

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
	
	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame) override;
	virtual void PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed) override;

	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;


public:
	void UpdateLight(float elapsed);
	void BuildDriftParticleObject(ID3D12GraphicsCommandList* cmdList);
	void DestroyDriftParticleObject();
	void UpdateLightConstants();
	void UpdateCameraConstant(int idx, Camera* camera);
	void UpdateVolumetricConstant();
	void UpdateConstants(const GameTimer& timer);
	void UpdateDynamicsWorld();

	void SetGraphicsCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex = 0);

	void SetComputeCBV(ID3D12GraphicsCommandList* cmdList);

	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex=0, bool cubeMapping=false);

	void OnPreciseKeyInput(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, float elapsed);

public:
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseUp(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual UI* GetUI() const override { return mpUI.get(); }

	virtual ID3D12RootSignature* GetRootSignature() const override { return mRootSignature.Get(); }

private:
	void BuildRootSignature();
	void BuildComputeRootSignature();
	void BuildGameObjects(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics);
	void BuildConstantBuffers();
	void BuildShadersAndPSOs(ID3D12GraphicsCommandList* cmdList);
	void BuildDescriptorHeap();

	void BuildCarObject(
		const XMFLOAT3& position,
		const XMFLOAT4& rotation,
		char color,
		bool isPlayer,
		ID3D12GraphicsCommandList* cmdList, 
		const std::shared_ptr<BulletWrapper>& dynamicsWorld, 
		UINT netID);

	void BuildMissileObject( 
		ID3D12GraphicsCommandList* cmdList, 
		const XMFLOAT3& position, int idx);

	void CreateVelocityMapViews();
	void CreateVelocityMapDescriptorHeaps();

	void CreateMsaaDescriptorHeaps();
	void CreateMsaaViews();

	void UpdateMissileObject();
	void UpdatePlayerObjects();

	void LoadWorldMap(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, const std::string& path);
	void LoadCheckPoint(ID3D12GraphicsCommandList* cmdList, const std::wstring& path);
	void LoadLights(ID3D12GraphicsCommandList* cmdList, const std::wstring& path);

	void SetMsaaQuality(UINT quality) { mMsaa4xQualityLevels = quality; }

private:
	std::unique_ptr<Camera> mMainCamera;
	std::unique_ptr<Camera> mDirectorCamera;
	Camera* mCurrentCamera;

	POINT mLastMousePos{};

	float mCameraRadius = 30.0f;

	LightConstants mMainLight;

	D3D12_CPU_DESCRIPTOR_HANDLE mMsaaVelocityMapRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE mMsaaVelocityMapSrvHandle;
	ComPtr<ID3D12Resource> mMsaaVelocityMap;

	ComPtr<ID3D12DescriptorHeap> mMsaaVelocityMapRtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mMsaaVelocityMapSrvDescriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE mMsaaRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE mMsaaSrvHandle;
	ComPtr<ID3D12Resource> mMsaaTarget;

	ComPtr<ID3D12DescriptorHeap> mMsaaRtvDescriptorHeap;

	std::unique_ptr<ConstantBuffer<CameraConstants>> mCameraCB;
	std::unique_ptr<ConstantBuffer<LightConstants>> mLightCB;
	std::unique_ptr<ConstantBuffer<GameInfoConstants>> mGameInfoCB;
	std::unique_ptr<ConstantBuffer<VolumetricConstants>> mVolumetricCB;

	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12RootSignature> mComputeRootSignature;

	std::map<std::string, std::vector<std::shared_ptr<Mesh>>> mMeshList;
	std::map<std::string, std::vector<std::shared_ptr<Texture>>> mTextureList;

	std::map<std::string, BoundingOrientedBox> mOOBBList;
	std::map<Layer, std::unique_ptr<Pipeline>> mPipelines;
	std::map<Layer, std::unique_ptr<ComputePipeline>> mPostProcessingPipelines;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	std::unique_ptr<ShadowMapRenderer> mShadowMapRenderer;

	Player* mPlayer = nullptr;
	std::array<std::shared_ptr<MissileObject>, MAX_ROOM_CAPACITY> mMissileObjects;
	std::array<std::shared_ptr<PhysicsPlayer>, MAX_ROOM_CAPACITY> mPlayerObjects;

	btDiscreteDynamicsWorld* mDynamicsWorld;

	bool mLODSet = false;
	bool mOutside = false;

	float mMissileInterval = 0.0f;
	float mCubemapInterval = 0.0f;

	UINT mCubemapDrawIndex = 0;

	std::vector<std::wstring> Texts;

	// Key pressed flag
	std::map<int, bool> mKeyMap;

	// set true when server start game.
	std::atomic_bool mGameStarted;

	// color map
	const std::array<XMFLOAT4, MAX_ROOM_CAPACITY> mColorMap = {
		(XMFLOAT4)Colors::Red, (XMFLOAT4)Colors::Blue,
		(XMFLOAT4)Colors::Green, (XMFLOAT4)Colors::Purple,
		(XMFLOAT4)Colors::Black, (XMFLOAT4)Colors::White,
		(XMFLOAT4)Colors::Orange, (XMFLOAT4)Colors::Yellow
	};

	// wheel offset
	const XMFLOAT3 mWheelOffset = { 1.25f, -0.2f, 1.85f };

	UINT mMsaa4xQualityLevels = 0;
	bool mMsaa4xEnable = false;

	bool mMotionBlurEnable = true;

	bool mCheckPointEnable = false;

	bool mBloomEnable = true;

	bool mVolumetricEnable = true;

	int32_t mDriftParticleEnable = false;

	btRigidBody* mTrackRigidBody = NULL;

	/*float mVolumetricOuter = 7.0f;
	float mVolumetricInner = 6.0f;
	float mVolumetricRange = 20.0f;*/
	
	std::vector<LightBundle> mLights;
	LightInfo mDirectionalLight;

	// Game end counter
	static const int WAIT_TO_REVERT = 5;
	std::atomic_bool mGameEnded = false;
	Clock::time_point mRevertTime;

	std::unique_ptr<InGameUI> mpUI;
};