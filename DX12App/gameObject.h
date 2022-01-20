#pragma once

#include "mesh.h"
#include "constantBuffer.h"
#include "camera.h"
#include "texture.h"

class GameScene;

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	void BuildSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

	virtual void Update(float elapsedTime, XMFLOAT4X4* parent);
	virtual void UpdateTransform(XMFLOAT4X4* parent);

	virtual void Draw(ID3D12GraphicsCommandList* cmdList,
		UINT rootMatIndex, UINT rootCbvIndex, UINT rootSrvIndex,
		UINT64 matGPUAddress, UINT64 byteOffset,
		const BoundingFrustum& viewFrustum, bool isSO=false);

	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList,
		UINT rootMatIndex, UINT rootCbvIndex, UINT rootSrvIndex,
		UINT64 matGPUAddress, UINT64 byteOffse, bool isSO=false);
	
	void UpdateBoudingBox();
	void Animate(float elapsedTime);

	void UpdateMatConstants(ConstantBuffer<MaterialConstants>* matCnst, int offset);

public:
	virtual void LoadModel(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList, 
		const std::wstring& path);
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

public:
	virtual void SetChild(GameObject* child);
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(const XMFLOAT3& pos);

	void SetLook(XMFLOAT3& look);
	void SetMesh(const std::shared_ptr<Mesh>& mesh) { mMeshes.push_back(mesh); }

	void SetRotation(XMFLOAT3& axis, float speed);
	void SetMovement(XMFLOAT3& dir, float speed);

	void SetReflected(XMFLOAT4& plane);
	void SetWorld(XMFLOAT4X4 world) { mWorld = world; }

	void SetCBVAddress(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mCbvGPUAddress = gpuHandle; }
	void SetSRVAddress(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mSrvGPUAddress = gpuHandle; }

public:
	virtual void PreDraw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtvResource, GameScene* scene) { }
	
	virtual void BuildDsvRtvView(
		ID3D12Device* device,
		ID3D12Resource* rtvResource,
		D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle) { }

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

	void Scale(float xScale, float yScale, float zScale);
	void Scale(const XMFLOAT3& scale);
	void Scale(float scale);

public:
	XMFLOAT3 GetPosition() const { return mPosition; }
	XMFLOAT3 GetRight() const { return mRight; }
	XMFLOAT3 GetLook() const { return mLook; }
	XMFLOAT3 GetUp() const { return mUp; }

	XMFLOAT4X4 GetWorld() const { return mWorld; }

	//UINT GetSRVIndex() const { return mSrvIndex; }
	UINT GetMeshCount() const { return (UINT)mMeshes.size(); }
	UINT GetTextureCount() const { return (UINT)mTextures.size(); }

	virtual ULONG GetCubeMapSize() const { return 0; }	
	virtual ObjectConstants GetObjectConstants();

	BoundingOrientedBox GetBoundingBox() const { return mOOBB; }	
	
	btRigidBody* GetRigidBody() { return mBtRigidBody; }

protected:
	XMFLOAT3 mPosition = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 mRight = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mLook = { 0.0f, 0.0f, 1.0f };
	XMFLOAT3 mScaling = { 1.0f, 1.0f, 1.0f };

	XMFLOAT4X4 mWorld = Matrix4x4::Identity4x4();

	btRigidBody* mBtRigidBody;
	std::vector<std::shared_ptr<Mesh>> mMeshes;
	std::vector<std::unique_ptr<Texture>> mTextures;

	D3D12_GPU_DESCRIPTOR_HANDLE mCbvGPUAddress{};
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGPUAddress{};

	BoundingOrientedBox mOOBB = { };

	GameObject* mParent = nullptr;
	GameObject* mChild = nullptr;
	GameObject* mSibling = nullptr;

	XMFLOAT3 mMoveDirection = {};
	XMFLOAT3 mRotationAxis = {};

	float mMoveSpeed = 0.0f;
	float mRotationSpeed = 0.0f;

	bool mReflected = false;
	XMFLOAT4X4 mReflectMatrix = Matrix4x4::Identity4x4();
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
	void BuildTerrainMesh(ID3D12Device* device,	ID3D12GraphicsCommandList* cmdList, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld,int blockWidth, int blockDepth);

public:
	float GetHeight(float x, float z) const;
	XMFLOAT3 GetNormal(float x, float z) const;

	int GetHeightMapWidth() const { return mHeightMapImage->GetWidth(); }
	int GetHeightMapDepth() const { return mHeightMapImage->GetDepth(); }
	
	void SetScale(const XMFLOAT3& scale) { mTerrainScale = scale; }
	XMFLOAT3 GetTerrainScale() const { return mTerrainScale; }

	float GetWidth() const { return mWidth * mTerrainScale.x; }
	float GetDepth() const { return mDepth * mTerrainScale.z; }

private:
	std::unique_ptr<HeightMapImage> mHeightMapImage;

	float* mHeightmapData = NULL;

	int mWidth = 0;
	int mDepth = 0;

	XMFLOAT3 mTerrainScale = { 1.0f,1.0f,1.0f };

	std::shared_ptr<btHeightfieldTerrainShape> mTerrainShape;
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
class DynamicCubeMapObject : public GameObject
{
public:
	DynamicCubeMapObject(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, LONG cubeMapSize);
	virtual ~DynamicCubeMapObject();

	virtual void BuildDsvRtvView(
		ID3D12Device* device,
		ID3D12Resource* rtvResource,
		D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle) override;

	void BuildCameras();

	virtual void PreDraw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtvResource, GameScene* scene) override;


public:
	virtual ULONG GetCubeMapSize() const { return mCubeMapSize; }

private:
	static const int RtvCounts = 6;

	ULONG mCubeMapSize = 0;

	std::array<std::unique_ptr<Camera>, RtvCounts> mCameras;

	D3D12_CPU_DESCRIPTOR_HANDLE mRtvCPUDescriptorHandles[RtvCounts];
	D3D12_CPU_DESCRIPTOR_HANDLE mDsvCPUDescriptorHandle;

	ComPtr<ID3D12Resource> mDepthStencilBuffer;

	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRect;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//

class MissileObject : public GameObject
{
public:
	MissileObject();
	~MissileObject();
	virtual void Update(float elapsedTime, XMFLOAT4X4* parent);
	void SetMesh(std::shared_ptr<Mesh> mesh, btVector3 forward, XMFLOAT3 position, std::shared_ptr<btDiscreteDynamicsWorld> dynamicsWorld);
	float GetDuration() { return mDuration; }
private:
	float mDuration = 3.0f;
};