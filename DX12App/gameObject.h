#pragma once

#include "mesh.h"
#include "constantBuffer.h"
#include "camera.h"
#include "texture.h"
#include "atomicFloat.h"
#include "interpolator.h"

class InGameScene;
class Pipeline;

enum class UPDATE_FLAG : char
{
	NONE = 0,
	CREATE,
	REMOVE
};

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	void BuildSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

	virtual void Update(float elapsedTime, int64_t clockDelta);

	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList,
		UINT rootMatIndex, UINT rootCbvIndex, UINT rootSrvIndex,
		UINT64 matGPUAddress, UINT64 byteOffse, bool isSO=false);

	void DrawInstanced(ID3D12GraphicsCommandList* cmdList,
		UINT rootMatIndex, UINT rootSBIndex, UINT rootSrvIndex,
		UINT64 matGPUAddress, UINT64 byteOffset, int InstanceCount, bool isSO = false);

	virtual void ChangeCurrentRenderTarget() {}

	void UpdateMatConstants(ConstantBuffer<MaterialConstants>* matCnst, int offset);
	void SortMeshes();
	void UpdateInverseWorld();
	void UpdateBoundingBox();

protected:
	virtual void UpdateTransform();

	void RotateDirectionVectors();
	void SetWorldByMotionState();
	void ResetTransformVectors();

	void Animate(float elapsedTime);

	void InterpolateRigidBody(float elapsed);
	void InterpolateWorldTransform(float elapsed, uint64_t clockDelta);

public:
	virtual void LoadModel(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList, 
		const std::wstring& path,
		bool collider = false);

	void LoadMaterial(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList,
		std::unordered_map<std::string, MatInfo>& mats, 
		const std::wstring& path);
	void LoadTexture(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const std::wstring& path,
		D3D12_SRV_DIMENSION dimension = D3D12_SRV_DIMENSION_TEXTURE2D);
	virtual bool LoadConvexHullShape(const std::wstring& path, const std::shared_ptr<BulletWrapper>& physics);
	virtual void BuildRigidBody(float mass, const std::shared_ptr<BulletWrapper>& physics);

	virtual void RemoveObject(btDiscreteDynamicsWorld& dynamicsWorld, Pipeline& pipeline);

public:
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(const XMFLOAT3& pos);

	void SetDiffuse(const std::string& name, const XMFLOAT4& color);

	void SetLook(XMFLOAT3& look);
	void SetMesh(const std::shared_ptr<Mesh>& mesh) { mMeshes.push_back(mesh); }

	void SetMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) { mMeshes = meshes; }
	void SetTextures(const std::vector<std::shared_ptr<Texture>>& textures) { mTextures = textures; }
	const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return mMeshes; }
	const std::vector<std::shared_ptr<Texture>>& GetTextures() const { return mTextures; }
	
	void CopyMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes);
	void SetBoudingBox(BoundingOrientedBox oobb);

	void SetBoudingBoxFromMeshes();

	void SetRotation(XMFLOAT3& axis, float speed);
	void SetMovement(XMFLOAT3& dir, float speed);

	void SetWorld(XMFLOAT4X4 world) { mWorld = world; }

	void SetName(std::string name) { mName = name; }
	std::string GetName() { return mName; }

	void SetCBVAddress(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mCbvGPUAddress = gpuHandle; }
	void SetSRVAddress(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mSrvGPUAddress = gpuHandle; }

	void SetRigidBody(btRigidBody* rigidbody) { mBtRigidBody = rigidbody; }

public:
	virtual void PreDraw(ID3D12GraphicsCommandList* cmdList, InGameScene* scene, const UINT& cubemapIndex) { }	
	virtual void BuildDsvRtvView(ID3D12Device* device) { }

public:
	virtual void Strafe(float dist, bool local=true);
	virtual void Upward(float dist, bool local=true);
	virtual void Walk(float dist, bool local=true);

	virtual void RotateY(float angle);
	virtual void Pitch(float angle);

	void Move(float dx, float dy, float dz);
	void Move(XMFLOAT3& dir, float dist);

	void Rotate(float pitch, float yaw, float roll);
	void Rotate(const XMFLOAT3& axis, float angle);

	void SetQuaternion(const XMFLOAT4& quaternion);
	void SetQuaternion(float x, float y, float z, float w);

	void Scale(float xScale, float yScale, float zScale);
	void Scale(const XMFLOAT3& scale);
	void Scale(float scale);

	void SetTransparent(bool transparent) { mTransparentOn = transparent; }

	void SetOOBBOrientation(const XMFLOAT4& quaternion) { mOOBB.Orientation = quaternion; }

public:
	const XMFLOAT3& GetPosition() const { return mPosition; }
	const XMFLOAT3& GetRight() const { return mRight; }
	const XMFLOAT3& GetLook() const { return mLook; }
	const XMFLOAT3& GetUp() const { return mUp; }
	const XMFLOAT4& GetQuaternion() const { return mQuaternion; }

	const XMFLOAT4X4& GetWorld() const { return mWorld; }
	const XMFLOAT4X4& GetInverseWorld() const { return mInvWorld; }

	const AtomicFloat3& GetLinearVelocity() { return mLinearVelocity; }

	//UINT GetSRVIndex() const { return mSrvIndex; }
	UINT GetMeshCount() const { return (UINT)mMeshes.size(); }
	UINT GetTextureCount() const { return (UINT)mTextures.size(); }

	virtual ULONG GetCubeMapSize() const { return 0; }	
	virtual ObjectConstants GetObjectConstants();
	virtual InstancingInfo GetInstancingInfo();
	const BoundingOrientedBox GetBoundingBox() const { return mOOBB; }
	
	btRigidBody* GetRigidBody() { return mBtRigidBody; }

	void ChangeUpdateFlag(UPDATE_FLAG expected, const UPDATE_FLAG& desired);
	void SetUpdateFlag(const UPDATE_FLAG& flag) { mUpdateFlag = flag; }

	void SetRimLight(bool rimlight) { mRimLightOn = rimlight; }

	UPDATE_FLAG GetUpdateFlag() const { return mUpdateFlag; }

	// test
	const XMFLOAT4& GetMeshDiffuse(const std::string& name);

protected:
	XMFLOAT3 mPosition = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 mRight = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mLook = { 0.0f, 0.0f, 1.0f };
	XMFLOAT3 mScaling = { 1.0f, 1.0f, 1.0f };
	XMFLOAT4 mQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };

	XMFLOAT4X4 mWorld = Matrix4x4::Identity4x4();
	XMFLOAT4X4 mOldWorld = Matrix4x4::Identity4x4();
	XMFLOAT4X4 mInvWorld = Matrix4x4::Identity4x4();
	XMFLOAT4X4 mRotation = Matrix4x4::Identity4x4();

	// Members for interpolation.
	std::mutex mProgressMut; // TEST
	float mCurrentTime = 0.0f;

	AtomicFloat3 mPrevOrigin;
	AtomicFloat4 mPrevQuat;

	AtomicFloat3 mCorrectionOrigin{};
	AtomicFloat4 mCorrectionQuat{};
	AtomicFloat3 mLinearVelocity{};

	Interpolator mInterpolator;
	// Members for interpolation.

	std::atomic<UPDATE_FLAG> mUpdateFlag;

	btRigidBody* mBtRigidBody = NULL;
	btCompoundShape* mBtCollisionShape = NULL;
	std::vector<std::shared_ptr<Mesh>> mMeshes;
	std::vector<std::shared_ptr<Texture>> mTextures;

	D3D12_GPU_DESCRIPTOR_HANDLE mCbvGPUAddress{};
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGPUAddress{};

	BoundingOrientedBox mOOBB = { };

	XMFLOAT3 mMoveDirection = {};
	XMFLOAT3 mRotationAxis = {};

	float mMoveSpeed = 0.0f;
	float mRotationSpeed = 0.0f;

	bool mReflected = false;
	XMFLOAT4X4 mReflectMatrix = Matrix4x4::Identity4x4();

	bool mCubemapOn = false;
	bool mMotionBlurOn = true;
	bool mRimLightOn = false;
	bool mTransparentOn = false;

	std::string mName;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class TerrainObject : public GameObject
{
public:
	TerrainObject(int width, int depth, const XMFLOAT3& scale = { 1.0f,1.0f,1.0f });
	TerrainObject(const TerrainObject& rhs) = delete;
	TerrainObject& operator=(const TerrainObject& rhs) = delete;
	virtual ~TerrainObject();

	void BuildHeightMap(const std::wstring& path);
	void BuildTerrainMesh(ID3D12Device* device,	ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics,int blockWidth, int blockDepth);

public:
	float GetHeight(float x, float z) const;
	XMFLOAT3 GetNormal(float x, float z) const;

	int GetHeightMapWidth() const { return mHeightMapImage->GetWidth(); }
	int GetHeightMapDepth() const { return mHeightMapImage->GetDepth(); }
	
	void SetScale(const XMFLOAT3& scale) { mTerrainScale = scale; }
	XMFLOAT3 GetTerrainScale() const { return mTerrainScale; }

	float GetWidth() const { return mWidth * mTerrainScale.x; }
	float GetDepth() const { return mDepth * mTerrainScale.z; }

	std::pair<float, float> GetBlockSize() { return std::make_pair(mBlockWidth * mTerrainScale.x, mBlockDepth * mTerrainScale.z); }

	std::vector<btRigidBody*> GetTerrainRigidBodies() { return mTerrainRigidBodies; }

private:
	std::unique_ptr<HeightMapImage> mHeightMapImage;

	int mWidth = 0;
	int mDepth = 0;

	int mBlockWidth = 0;
	int mBlockDepth = 0;

	XMFLOAT3 mTerrainScale = { 1.0f,1.0f,1.0f };

	std::vector<btRigidBody*> mTerrainRigidBodies;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class Billboard : public GameObject
{
public:
	Billboard(float width, float height);
	virtual ~Billboard();

	void AppendBillboard(const XMFLOAT3& pos);
	void BuildMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

	void SetDurationTime(std::chrono::milliseconds& time);
	bool IsTimeOver(std::chrono::steady_clock::time_point& currentTime);

private:
	float mWidth = 0.0f;
	float mHeight = 0.0f;

	std::vector<BillboardVertex> mVertices;
	std::vector<UINT> mIndices;
	
	std::chrono::steady_clock::time_point mCreationTime;
	std::chrono::milliseconds mDurationTime;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//

class SOParticleObject : public GameObject
{
public:
	SOParticleObject(GameObject& parent);
	virtual ~SOParticleObject() = default;
	virtual void Update(float elapsedTime, int64_t clockDelta) override;

	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList,
		UINT rootMatIndex, UINT rootCbvIndex, UINT rootSrvIndex,
		UINT64 matGPUAddress, UINT64 byteOffset, bool isSO = false);

	void SetLocalOffset(XMFLOAT3 offset);

	void SetParticleEnable(bool enable) { mParticleEnable = enable; }

private:
	GameObject& mParent;
	XMFLOAT3 mLocalOffset = { 0.0f, 0.0f, 0.0f };
	int32_t mParticleEnable = false;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

class MissileObject : public GameObject
{
public:
	MissileObject(const XMFLOAT3& position);
	virtual ~MissileObject();

	virtual void Update(float elapsedTime, int64_t clockDelta) override;

	void SetMesh(const std::shared_ptr<Mesh>& mesh, btVector3 forward, XMFLOAT3 position, std::shared_ptr<BulletWrapper> physics);
	float GetDuration() { return mDuration; }

	void SetCorrectionTransform(SC::packet_missile_transform* pck, uint64_t timePoint, float latency);

	void SetParticle(const std::shared_ptr<SOParticleObject>& particle) { mParticle = particle; }
	std::shared_ptr<SOParticleObject> GetParticle() { return mParticle; }

public:
	void SetActive(bool state);
	bool IsActive() const { return mActive; }

private:
	float mDuration = 3.0f;
	std::atomic_bool mActive;
	std::shared_ptr<SOParticleObject> mParticle;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

class StaticObject : public GameObject
{
public:
	StaticObject() = default;
	virtual ~StaticObject() = default;

	virtual void Update(float elapsedTime, int64_t clockDelta) override;
};