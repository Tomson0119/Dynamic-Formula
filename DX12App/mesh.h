#pragma once

#include "heightMapImage.h"
#include "vertex.h"

class Texture;

struct MatInfo
{
	Material Mat;
	XMFLOAT4X4 TexTransform;
	XMFLOAT2 TexOffset;
	XMFLOAT2 TexScale;
	int SrvIndex;

	MatInfo()
		: Mat{},
		  SrvIndex(-1)
	{
		TexTransform = Matrix4x4::Identity4x4();
		TexScale = { 1.0f, 1.0f };
		TexOffset = { 0.0f,0.0f };
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class Mesh 
{
public:
	Mesh();
	virtual ~Mesh() { }

	void CreateResourceInfo(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		UINT vbStride, UINT ibStride,
		D3D12_PRIMITIVE_TOPOLOGY topology,
		const void* vbData, UINT vbCount,
		const void* ibData, UINT ibCount);

	virtual void PrepareBufferViews(ID3D12GraphicsCommandList* cmdList, bool isSO) { }
	virtual void Draw(ID3D12GraphicsCommandList* cmdList, bool isSO=false);

	void LoadMesh(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList, 
		std::ifstream& file,
		const std::vector<XMFLOAT3>& positions,
		const std::vector<XMFLOAT3>& normals,
		const std::vector<XMFLOAT2>& texcoords,
		const MatInfo& mat);

	void SetMaterial(Material mat) { mMaterial.Mat = mat; }
	void SetSrvIndex(UINT idx) { mMaterial.SrvIndex = idx; }

public:
	MaterialConstants GetMaterialConstant() const;
	UINT GetSrvIndex() const { return mMaterial.SrvIndex; }

protected:
	ComPtr<ID3D12Resource> mVertexBufferGPU;
	ComPtr<ID3D12Resource> mIndexBufferGPU;

	ComPtr<ID3D12Resource> mVertexUploadBuffer;
	ComPtr<ID3D12Resource> mIndexUploadBuffer;

	D3D12_PRIMITIVE_TOPOLOGY mPrimitiveTopology = {};

	UINT mSlot = 0;
	UINT mVerticesCount = 0;
	UINT mVertexStride = 0;
	UINT mIndexCount = 0;
	UINT mStartIndex = 0;
	UINT mBaseVertex = 0;

	MatInfo mMaterial = {};

public:
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView = {};
	
	BoundingOrientedBox mOOBB = {};
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class BoxMesh : public Mesh
{
public:
	BoxMesh(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		float width, float height, float depth);
	virtual ~BoxMesh() { }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class GridMesh : public Mesh
{
public:
	GridMesh(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		float width, float height, float u, float v);
	virtual ~GridMesh() { }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class SphereMesh : public Mesh
{
public:
	SphereMesh(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		float radius, int sliceCount, int stackCount);
	virtual ~SphereMesh() { }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class HeightMapGridMesh : public Mesh
{
public:
	HeightMapGridMesh(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		int xStart, int zStart,
		int width, int depth,
		const XMFLOAT3& scale,
		HeightMapImage* context);

	virtual ~HeightMapGridMesh() { }

	float GetHeight(int x, int z, HeightMapImage* context) const;
	XMFLOAT4 GetColor(int x, int z, HeightMapImage* context) const;
	
private:
	XMFLOAT3 mScale = {};

	int mWidth = 0;
	int mDepth = 0;	
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class HeightMapPatchListMesh : public Mesh
{
public:
	HeightMapPatchListMesh(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		int xStart, int zStart,
		int width, int depth,
		const XMFLOAT3& scale,
		HeightMapImage* context);

	virtual ~HeightMapPatchListMesh() { }

	float GetHeight(int x, int z, HeightMapImage* context) const;
	std::pair<float, float> GetMinMax() { return std::pair<float, float>(mMinHeight, mMaxHeight); }; // min, max

private:
	XMFLOAT3 mScale = {};

	int mWidth = 0;
	int mDepth = 0;

	float mMinHeight = FLT_MAX;
	float mMaxHeight = -FLT_MAX;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class ParticleMesh : public Mesh
{
public:
	ParticleMesh(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const XMFLOAT3& position,
		const XMFLOAT2& size,
		const XMFLOAT3& direction,
		float lifeTime,
		float speed,
		int maxParticle);

	virtual ~ParticleMesh() { }

	virtual void PrepareBufferViews(ID3D12GraphicsCommandList* cmdList, bool isSO) override;
	virtual void Draw(ID3D12GraphicsCommandList* cmdList, bool isSO=false) override;

private:
	void CreateStreamOutputBuffer(
		ID3D12Device* device,	
		ID3D12GraphicsCommandList* cmdList);

	void CopyToRenderBuffer(ID3D12GraphicsCommandList* cmdList);

private:
	ComPtr<ID3D12Resource> mStreamOutputBuffer;
	ComPtr<ID3D12Resource> mRenderBuffer;

	ComPtr<ID3D12Resource> mDefaultBuffer;
	ComPtr<ID3D12Resource> mUploadBuffer;
	
	ComPtr<ID3D12QueryHeap> mQueryHeap;
	ComPtr<ID3D12Resource> mQueryBuffer;
	D3D12_QUERY_DATA_SO_STATISTICS *mQueryStatistics;

	UINT64* mUploadBufferFilledSize;

	D3D12_STREAM_OUTPUT_BUFFER_VIEW mSOBufferView;

	int mMaxParticle;
	bool mStart;
};