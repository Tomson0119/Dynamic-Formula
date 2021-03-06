#pragma once

#include "gameObject.h"
#include "atomicFloat.h"

class Camera;
class InGameScene;

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
	virtual float GetCurrentVelocity() { return 0.0f; }

	virtual void SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex) {};
	virtual void Update(float elapsedTime, int64_t clockDelta) override;
	virtual void OnPlayerUpdate(float elapsedTime) final { }
	virtual void OnCameraUpdate(float elapsedTime) { }
	virtual std::shared_ptr<btRaycastVehicle> GetVehicle() { return NULL; }

	virtual void SetItemNum(int num) { }
	virtual int GetItemNum() { return 0; }
	virtual void SetBooster() { }
	virtual void SetRimLight(bool rimlight) { }

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
};

/////////////////////////////////////////////////////////////////////////////////////
//

class WheelObject : public GameObject
{
public:
	WheelObject(GameObject& parent);
	virtual ~WheelObject();

	void SetLocalOffset(const XMFLOAT3& offset) { mLocalOffset = offset; }

	void UpdatePosition(float Elapsed, const btTransform& wheelTransform);
	void SetSteeringAngle(float angle);

	virtual void Update(float elapsedTime, int64_t clockDelta) override;

private:
	GameObject& mParent;
	XMFLOAT3 mLocalOffset;
	float mSteeringAngle;
	bool mIsStandAlone;
};

class PhysicsPlayer : public Player
{
public:
	PhysicsPlayer(UINT netID);
	virtual ~PhysicsPlayer();

	virtual void OnCameraUpdate(float elapsedTime);
	virtual void Update(float elapsedTime, int64_t clockDelta) override;
	virtual void OnPreciseKeyInput(float Elapsed) override;
	virtual void SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex);
	virtual Camera* ChangeCameraMode(int cameraMode);
	
	virtual void BuildDsvRtvView(ID3D12Device* device) override;
	virtual void BuildRigidBody(const std::shared_ptr<BulletWrapper>& physics);

	virtual void PreDraw(ID3D12GraphicsCommandList* cmdList, InGameScene* scene, const UINT& cubemapIndex) override;
	virtual void ChangeCurrentRenderTarget() { mCurrentRenderTarget = 1 - mCurrentRenderTarget; }

	virtual void RemoveObject(btDiscreteDynamicsWorld& dynamicsWorld, Pipeline& pipeline) override;
	
	virtual void UpdateFrontLight();

	virtual void SetBooster() { mBoosterLeft = mBoosterTime; }
	virtual void SetRimLight(bool rimlight) { mRimLightOn = rimlight; }
public:
	void SetSpawnTransform(SC::packet_spawn_transform* pck);

	void SetMesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Mesh>& wheelMesh, std::shared_ptr<BulletWrapper> physics);
	void SetMesh(const std::shared_ptr<Mesh>& Mesh);
	void SetWheel(std::shared_ptr<WheelObject> wheel, int index) { mWheel[index] = wheel; }
	void SetInvincibleOn(int duration);

	void BuildCameras();

	void SetCorrectionTransform(SC::packet_player_transform* pck, uint64_t timePoint, float latency);

private:
	void UpdateInvincibleState(float elapsed);

public:
	virtual std::shared_ptr<btRaycastVehicle> GetVehicle() { return mVehicle; }
	virtual UINT GetNetID() { return mNetID; }

	WheelObject* GetWheel(int index) { return mWheel[index].get(); }
	virtual float GetCurrentVelocity() { return mCurrentSpeed; }

	virtual void SetItemNum(int num) { mItemNum = num; }
	virtual int GetItemNum() { return mItemNum; }
	virtual float GetDriftGauge() { return mDriftGauge; }

	virtual ULONG GetCubeMapSize() const { return mCubeMapSize; }

	LightBundle* GetLightBundle() { return mFrontLight; }

	void SetCurrentSpeed(int speed) { mCurrentSpeed = speed; }
	int GetCurrentSpeed() { return mCurrentSpeed; }

private:
	static const int RtvCounts = 12;
	static const float TransparentInterval;
	
	const float mWheelFriction = 5.0f;
	const float mWheelDriftFriction = 0.0f;

	ULONG mCubeMapSize = 500;

	std::array<std::unique_ptr<Camera>, RtvCounts / 2> mCameras;

	D3D12_CPU_DESCRIPTOR_HANDLE mRtvCPUDescriptorHandles[RtvCounts]{};
	D3D12_CPU_DESCRIPTOR_HANDLE mDsvCPUDescriptorHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE mSrvCPUDescriptorHandle{};

	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	std::unique_ptr<Texture> mCubeMap[2];

	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRect;

	ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

	UINT mCurrentRenderTarget = 0;

	UINT mNetID = -1;

	std::shared_ptr<WheelObject> mWheel[4];
	btRaycastVehicle::btVehicleTuning mTuning;
	std::shared_ptr<btVehicleRaycaster> mVehicleRayCaster;
	std::shared_ptr<btRaycastVehicle> mVehicle;

	float mBoosterLeft = 0.0f;
	float mBoosterTime = 5.0f;

	float mEngineForce = 0.f;

	float mMaxEngineForce = 2000.f;
	const float mBoosterEngineForce = 6000.f;
	const float mBaseEngineForce = 2000.f;

	float mVehicleSteering = 0.f;
	float mSteeringIncrement = 5.0f;
	float mSteeringClamp = 0.15f;

	std::atomic_int mCurrentSpeed = 0;
	float mMaxSpeed = 350.0f;

	float mBreakingForce = 0.0f;

	float mFovCoefficient = 1.0f;

	std::atomic_int mItemNum = 0;
	float mDriftGauge = 0.0f;

	std::atomic_bool mSpawnFlag = false;
	AtomicFloat3 mSpawnPosition;
	AtomicFloat4 mSpawnRotation;

	std::atomic_bool mInvincibleOnFlag = false;
	std::atomic_int mInvincibleInterval = 0;
	
	bool mInvincible = false;
	float mInvincibleDuration = 0.0f;
	float mTransparentTime = TransparentInterval;

	XMFLOAT3 mLightOffset[2] = { {1.0f, 0.0f, 1.5f}, {-1.0f, 0.0f, 1.5f} };
	LightBundle mFrontLight[2];
};
