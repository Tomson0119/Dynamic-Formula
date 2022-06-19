#pragma once

#include "Shader.h"
#include "gameObject.h"

enum class Layer : int
{
	SkyBox,
	Terrain,
	NormalMapped,
	Default,
	Mirror,
	Reflected,
	Billboard,
	ShadowDebug,
	DynamicCubeMap,
	MotionBlur,
	Bloom,
	VolumetricScattering,
	Instancing,
	Color,
	CheckPoint,
	Transparent,
	DriftParticle,
	MissileParticle
};

enum class DrawType : int
{
	Instancing,
	Shadow_First,
	Shadow_Second,
	Shadow_Third,
	CubeMapping,
	Common
};

class Pipeline
{
public:
	Pipeline();
	Pipeline(const Pipeline& rhs) = delete;
	Pipeline& operator=(const Pipeline& rhs) = delete;
	virtual ~Pipeline();

	virtual void BuildPipeline(
		ID3D12Device* device, 
		ID3D12RootSignature* rootSig,
		Shader* shader=nullptr);

	virtual void BuildDescriptorHeap(ID3D12Device* device, UINT matIndex, UINT cbvIndex, UINT srvIndex);

	virtual void BuildConstantBuffer(ID3D12Device* device);
	void BuildCBV(ID3D12Device* device);
	void BuildSRV(ID3D12Device* device);

	void SetWiredFrame(bool wired) { mIsWiredFrame = wired; }
	void SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) { mPrimitive = topology; }
	void SetCullClockwise();

	void SetAlphaBlending();
	void SetStencilOp(
		UINT stencilRef, D3D12_DEPTH_WRITE_MASK depthWriteMask,
		D3D12_STENCIL_OP stencilFail, D3D12_STENCIL_OP stencilDepthFail,
		D3D12_STENCIL_OP stencilPass, D3D12_COMPARISON_FUNC stencilFunc,
		UINT8 rtWriteMask);

	void AppendObject(const std::shared_ptr<GameObject>& obj);

	void DeleteObject(int idx);
	std::vector<std::shared_ptr<GameObject>>::iterator DeleteObject(std::vector<std::shared_ptr<GameObject>>::iterator iter);
	void ResetPipeline(ID3D12Device* device);

	void PreparePipeline(ID3D12GraphicsCommandList* cmdList, bool drawWiredFrame = false, bool setPipeline = true, bool msaaOff = false);

	void SetMsaa(bool msaaEnable, UINT msaaQuality) { mMsaa4xQualityLevels = msaaQuality; mMsaaEnable = msaaEnable; }

	void SortMeshes();

	void Update(float elapsed, int64_t clockDelta, Camera* camera);
	virtual void SetAndDraw(ID3D12GraphicsCommandList* cmdList, bool drawWiredFrame=false, bool setPipeline=true, bool msaaOff=false, DrawType type = DrawType::Common);
	virtual void Draw(ID3D12GraphicsCommandList* cmdList, bool isSO = false, DrawType type = DrawType::Common);

	virtual void UpdateConstants(Camera* camera, DrawType type, bool culling = false);

	std::vector<std::shared_ptr<GameObject>>& GetRenderObjects() { return mRenderObjects; }

protected:
	// 0번 : MSAA 렌더
	// 1번 : MSAA 와이어프레임 렌더
	// 2번 : MSAA 사용하지 않고 렌더
	ComPtr<ID3D12PipelineState> mPSO[3];
	ComPtr<ID3D12DescriptorHeap> mCbvSrvDescriptorHeap;

	D3D12_RASTERIZER_DESC	  mRasterizerDesc   = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	D3D12_BLEND_DESC		  mBlendDesc		= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	D3D12_DEPTH_STENCIL_DESC  mDepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	DXGI_FORMAT mBackBufferFormat   = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT mVelocityMapFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	D3D12_PRIMITIVE_TOPOLOGY_TYPE mPrimitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	std::unique_ptr<ConstantBuffer<ObjectConstants>> mObjectCB;
	std::unique_ptr<ConstantBuffer<MaterialConstants>> mMaterialCB;

	std::vector<std::shared_ptr<GameObject>> mRenderObjects;
	
	UINT mRootParamMatIndex = 0;
	UINT mRootParamCBVIndex = 0;
	UINT mRootParamSRVIndex = 0;

	UINT mStencilRef = 0;
	bool mIsWiredFrame = false;

	UINT mMsaa4xQualityLevels = 0;
	bool mMsaaEnable = false;
};

//////////////////////////////////////////////////////////////////////////////////
//
class SkyboxPipeline : public Pipeline
{
public:
	SkyboxPipeline(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, char mapIndex = 0);
	virtual ~SkyboxPipeline();

	virtual void BuildPipeline(
		ID3D12Device* device,
		ID3D12RootSignature* rootSig,
		Shader* shader = nullptr) override;
};


//////////////////////////////////////////////////////////////////////////////////
//
class StreamOutputPipeline : public Pipeline
{
public:
	StreamOutputPipeline(UINT objectMaxCount);
	virtual ~StreamOutputPipeline();

	virtual void BuildPipeline(
		ID3D12Device* device,
		ID3D12RootSignature* rootSig,
		Shader* shader = nullptr) override;

	virtual void SetAndDraw(
		ID3D12GraphicsCommandList* cmdList,
		bool drawWiredFrame = false,
		bool setPipeline = true, bool msaaOff = false, DrawType type = DrawType::Common) override;

	virtual void BuildDescriptorHeap(ID3D12Device* device, UINT matIndex, UINT cbvIndex, UINT srvIndex);

	virtual void BuildConstantBuffer(ID3D12Device* device);

	void AppendObject(ID3D12Device* device, const std::shared_ptr<GameObject>& obj);

private:
	void BuildSOPipeline(
		ID3D12Device* device,
		ID3D12RootSignature* rootSig,
		Shader* shader = nullptr);

	void CreateStreamOutputDesc();

	virtual void BuildCBV(ID3D12Device* device);

	virtual void BuildSRV(ID3D12Device* device);

private:
	D3D12_STREAM_OUTPUT_DESC mStreamOutputDesc;
	std::vector<D3D12_SO_DECLARATION_ENTRY> mSODeclarations;
	std::vector<UINT> mStrides;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> mConstantBufferGPUHandles;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> mShaderResourceGPUHandles;

	UINT mObjectMaxCount = 0;
};

//////////////////////////////////////////////////////////////////////////////////
//
class InstancingPipeline : public Pipeline
{
public:
	InstancingPipeline();
	virtual ~InstancingPipeline();
	virtual void Draw(ID3D12GraphicsCommandList* cmdList, bool isSO = false, DrawType type = DrawType::Common);
	virtual void BuildConstantBuffer(ID3D12Device* device);
	virtual void UpdateConstants(Camera* camera, DrawType type, bool culling = false);

	std::map<std::string, int> mInstancingCount;
private:
	std::unique_ptr<StructuredBuffer<InstancingInfo>> mInstancingSB[5];
	UINT mRootParamSBIndex = 9;
};


//////////////////////////////////////////////////////////////////////////////////
//
class ComputePipeline
{
public:
	ComputePipeline();
	virtual ~ComputePipeline();

	virtual void BuildPipeline(
		ID3D12Device* device,
		ID3D12RootSignature* rootSig,
		ComputeShader* shader = nullptr, bool init = false);

	virtual void Dispatch(ID3D12GraphicsCommandList* cmdList);

	virtual void SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn = false) {}

	virtual void CopyRTToMap(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* source,
		ID3D12Resource* dest) {}

	virtual void ResolveRTToMap(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* source,
		ID3D12Resource* dest, DXGI_FORMAT format) {}

	virtual void CopyMapToRT(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtBuffer) {}

	virtual void CopyCurrentToPreviousBuffer(ID3D12GraphicsCommandList* cmdList) {}

protected:
	std::vector<ComPtr<ID3D12PipelineState>> mPSOs;
	ID3D12RootSignature* mComputeRootSig;
};

class MotionBlurPipeline : public ComputePipeline
{
public:
	MotionBlurPipeline();
	virtual ~MotionBlurPipeline();

	virtual void BuildPipeline(
		ID3D12Device* device,
		ID3D12RootSignature* rootSig,
		ComputeShader* shader = nullptr, bool init = false);

	virtual void SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn = false);

	virtual void CopyRTToMap(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* source,
		ID3D12Resource* dest);

	virtual void ResolveRTToMap(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* source,
		ID3D12Resource* dest, DXGI_FORMAT format);

	virtual void CopyMapToRT(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtBuffer);

	virtual void CopyCurrentToPreviousBuffer(ID3D12GraphicsCommandList* cmdList);

	virtual void Dispatch(ID3D12GraphicsCommandList* cmdList);

	virtual void CreateTextures(ID3D12Device* device);
	virtual void BuildDescriptorHeap(ID3D12Device* device);
	virtual void BuildSRVAndUAV(ID3D12Device* device);

protected:
	static const int InputCount = 3;

	ComPtr<ID3D12DescriptorHeap> mSrvUavDescriptorHeap;

	std::unique_ptr<Texture> mInputTexture[InputCount];
	std::unique_ptr<Texture> mOutputTexture;
};

#define GAUSSIAN_RADIUS 7

class BloomPipeline : public MotionBlurPipeline
{
public:
	BloomPipeline();
	virtual ~BloomPipeline();

	virtual void SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn = false);

	virtual void Dispatch(ID3D12GraphicsCommandList* cmdList);

	virtual void CreateTextures(ID3D12Device* device);
	virtual void BuildDescriptorHeap(ID3D12Device* device);
	virtual void BuildSRVAndUAV(ID3D12Device* device);

	virtual void CopyMapToRT(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtBuffer);

private:
	std::unique_ptr<Texture> mProcessingTexture[3];

	float mBlurCoefficients[GAUSSIAN_RADIUS + 1];
};

class VolumetricScatteringPipeline : public MotionBlurPipeline
{
public:
	VolumetricScatteringPipeline();
	virtual ~VolumetricScatteringPipeline();

	virtual void SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn);

	virtual void Dispatch(ID3D12GraphicsCommandList* cmdList);

	virtual void CreateTextures(ID3D12Device* device);
	virtual void BuildDescriptorHeap(ID3D12Device* device);
	virtual void BuildSRVAndUAV(ID3D12Device* device);
	virtual void ResolveRTToMap(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource* dest, DXGI_FORMAT format);
};