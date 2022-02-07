#pragma once

#include "gameObject.h"
#include "camera.h"
#include "inGameScene.h"

class Player : public GameObject
{
public:
	Player();
	Player(const Player& rhs) = delete;
	Player& operator=(const Player& rhs) = delete;
	virtual ~Player();

public:
	void Walk(float dist, bool updateVelocity=true);
	void Strafe(float dist, bool updateVelocity=true);
	void Upward(float dist, bool updateVelocity=true);

	virtual void OnPreciseKeyInput(float Elapsed) { };

	void Move(XMFLOAT3& shift, bool updateVelocity);

	virtual void RotateY(float angle) override;
	virtual void Pitch(float angle) override;

	void SetCamera(Camera* camera) { mCamera = camera; }
	void SetPlayerContext(void* context) { mPlayerUpdateContext = context; }
	void SetCameraContext(void* context) { mCameraUpdateContext = context; }

	void SetVelocity(const XMFLOAT3& vel) { mVelocity = vel; }
	void SetGravity(const XMFLOAT3& grav) { mGravity = grav; }

	XMFLOAT3 GetVelocity() const { return mVelocity; }
	XMFLOAT3 GetGravity() const { return mGravity; }

public:
	virtual Camera* ChangeCameraMode(int cameraMode);

	virtual void SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex) {};
	virtual void Update(float elapsedTime, XMFLOAT4X4* parent) override;
	virtual void OnPlayerUpdate(float elapsedTime) { }
	virtual void OnCameraUpdate(float elapsedTime) { }
	virtual std::shared_ptr<btRaycastVehicle> GetVehicle() { return NULL; }
protected:
	XMFLOAT3 mVelocity = {};
	XMFLOAT3 mGravity = {};
	
	float mMaxVelocityXZ = 0.0f;
	float mMaxVelocityY = 0.0f;
	float mFriction = 0.0f;

	void* mPlayerUpdateContext = nullptr;
	void* mCameraUpdateContext = nullptr;

	Camera* mCamera = nullptr;
};


/////////////////////////////////////////////////////////////////////////////////////
//
class TerrainPlayer : public Player
{
public:
	TerrainPlayer(void* context);
	TerrainPlayer(const TerrainPlayer& rhs) = delete;
	TerrainPlayer& operator=(const TerrainPlayer& rhs) = delete;
	virtual ~TerrainPlayer();

	virtual Camera* ChangeCameraMode(int cameraMode) override;

	virtual void OnPlayerUpdate(float elapsedTime) override;
	virtual void OnCameraUpdate(float elapsedTime) override;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class WheelObject : public GameObject
{
public:
	WheelObject();
	virtual ~WheelObject();

	void UpdateRigidBody(float Elapsed, btTransform wheelTransform);
};

class PhysicsPlayer : public Player
{
public:
	PhysicsPlayer(UINT netID);
	virtual ~PhysicsPlayer();

	virtual void OnCameraUpdate(float elapsedTime);
	virtual void OnPlayerUpdate(float elapsedTime);
	virtual void Update(float elapsedTime, XMFLOAT4X4* parent) override;
	virtual void OnPreciseKeyInput(float Elapsed);
	virtual void UpdateTransform(XMFLOAT4X4* parent) { }
	virtual void SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex);
	virtual Camera* ChangeCameraMode(int cameraMode);
	virtual std::shared_ptr<btRaycastVehicle> GetVehicle() { return mVehicle; }
	virtual UINT GetNetID() { return mNetID; }

	void SetMesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Mesh>& wheelMesh, std::shared_ptr<BulletWrapper> physics);
	void SetWheel(WheelObject* wheel, int index) { mWheel[index] = wheel; }
	void BuildRigidBody(std::shared_ptr<BulletWrapper> physics);

private:
	WheelObject* mWheel[4];
	btRaycastVehicle::btVehicleTuning mTuning;
	std::shared_ptr<btVehicleRaycaster> mVehicleRayCaster;
	std::shared_ptr<btRaycastVehicle> mVehicle;

	float mBoosterLeft = 0.0f;
	float mBoosterTime = 5.0f;

	float mEngineForce = 0.f;
	float mBreakingForce = 0.f;

	float mMaxEngineForce = 8000.f;
	float mBoosterEngineForce = 300000.f;

	float mVehicleSteering = 0.f;
	float mSteeringIncrement = 0.01f;
	float mSteeringClamp = 0.5f;

	float mCurrentSpeed = 0.0f;
	float mMaxSpeed = 1000.0f;

	float mFovCoefficient = 1.0f;

public:
	virtual void BuildDsvRtvView(
		ID3D12Device* device) override;

	void BuildCameras();

	virtual void PreDraw(ID3D12GraphicsCommandList* cmdList, InGameScene* scene, const UINT& cubemapIndex) override;
	virtual void ChangeCurrentRenderTarget() { mCurrentRenderTarget = 1 - mCurrentRenderTarget; }

public:
	virtual ULONG GetCubeMapSize() const { return mCubeMapSize; }

private:
	static const int RtvCounts = 12;

	ULONG mCubeMapSize = 500;

	std::array<std::unique_ptr<Camera>, RtvCounts / 2> mCameras;

	D3D12_CPU_DESCRIPTOR_HANDLE mRtvCPUDescriptorHandles[RtvCounts];
	D3D12_CPU_DESCRIPTOR_HANDLE mDsvCPUDescriptorHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE mSrvCPUDescriptorHandle;

	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	std::unique_ptr<Texture> mCubeMap[2];

	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRect;

	ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

	UINT mCurrentRenderTarget = 0;

	UINT mNetID = -1;
};
