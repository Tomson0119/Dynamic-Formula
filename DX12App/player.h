#pragma once

#include "gameObject.h"
#include "camera.h"
#include "inGameScene.h"

enum class UPDATE_FLAG : char
{
	NONE = 0,
	UPDATE,
	REMOVE
};

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

	void ChangeUpdateFlag(UPDATE_FLAG expected, UPDATE_FLAG desired);
	void SetUpdateFlag(UPDATE_FLAG flag) { mUpdateFlag = flag; }
	UPDATE_FLAG GetUpdateFlag() const { return mUpdateFlag; }

public:
	virtual Camera* ChangeCameraMode(int cameraMode);
	virtual float GetCurrentVelocity() { return 0.0f; }

	virtual void SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex) {};
	virtual void Update(float elapsedTime) override;
	virtual void OnPlayerUpdate(float elapsedTime) { }
	virtual void OnCameraUpdate(float elapsedTime) { }
	virtual std::shared_ptr<btRaycastVehicle> GetVehicle() { return NULL; }

	virtual int GetItemNum() { return 0; }
	virtual float GetDriftGauge() { return 0.0f; }

protected:
	XMFLOAT3 mVelocity = {};
	XMFLOAT3 mGravity = {};
	
	float mMaxVelocityXZ = 0.0f;
	float mMaxVelocityY = 0.0f;
	float mFriction = 0.0f;

	void* mPlayerUpdateContext = nullptr;
	void* mCameraUpdateContext = nullptr;

	Camera* mCamera = nullptr;

	std::atomic<UPDATE_FLAG> mUpdateFlag;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class WheelObject : public GameObject
{
public:
	WheelObject();
	virtual ~WheelObject();

	void UpdateRigidBody(const float& Elapsed, const btTransform& wheelTransform);
};

class PhysicsPlayer : public Player
{
public:
	PhysicsPlayer(UINT netID);
	virtual ~PhysicsPlayer();

	virtual void UpdateTransform() override;

	virtual void OnCameraUpdate(float elapsedTime);
	virtual void OnPlayerUpdate(float elapsedTime);
	virtual void Update(float elapsedTime) override;
	virtual void OnPreciseKeyInput(float Elapsed);
	virtual void SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex);
	virtual Camera* ChangeCameraMode(int cameraMode);
	virtual std::shared_ptr<btRaycastVehicle> GetVehicle() { return mVehicle; }
	virtual UINT GetNetID() { return mNetID; }

	void SetMesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Mesh>& wheelMesh, std::shared_ptr<BulletWrapper> physics);
	void SetMesh(const std::shared_ptr<Mesh>& Mesh);
	void SetWheel(std::shared_ptr<WheelObject> wheel, int index) { mWheel[index] = wheel; }
	//void BuildRigidBody(std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);

	std::shared_ptr<WheelObject> GetWheel(int index) { return mWheel[index]; }
	virtual float GetCurrentVelocity() { return mCurrentSpeed; }

	//void BuildRigidBody(std::shared_ptr<BulletWrapper> physics);
	virtual void BuildRigidBody(const std::shared_ptr<BulletWrapper>& physics);

	void InterpolateTransform(float elapsed, float latency);
	void SetCorrectionTransform(SC::packet_player_transform* pck, float latency);

	virtual int GetItemNum() { return mItemNum; }
	virtual float GetDriftGauge() { return mDriftGauge; }

private:
	std::shared_ptr<WheelObject> mWheel[4];
	btRaycastVehicle::btVehicleTuning mTuning;
	std::shared_ptr<btVehicleRaycaster> mVehicleRayCaster;
	std::shared_ptr<btRaycastVehicle> mVehicle;

	AtomicInt3 mCorrectionOrigin{};
	AtomicInt4 mCorrectionQuat{};

	const float mInterpSpeed = 5.0f;

	float mBoosterLeft = 0.0f;
	float mBoosterTime = 5.0f;

	float mEngineForce = 0.f;

	float mMaxEngineForce = 8000.f;
	float mBoosterEngineForce = 300000.f;

	float mVehicleSteering = 0.f;
	float mSteeringIncrement = 8.0f;
	float mSteeringClamp = 0.5f;

	float mCurrentSpeed = 0.0f;
	float mMaxSpeed = 1000.0f;

	float mFovCoefficient = 1.0f;

	int mItemNum = 0;
	float mDriftGauge = 0.0f;

public:
	virtual void BuildDsvRtvView(ID3D12Device* device) override;

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
