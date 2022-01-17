#pragma once

#include "pipeline.h"

class GameScene;

class ShadowMapRenderer : public Pipeline
{
public:
	ShadowMapRenderer(ID3D12Device* device, UINT width, UINT height, UINT lightCount);
	virtual ~ShadowMapRenderer();

	virtual void BuildPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, Shader* shader=nullptr) override;
	virtual void BuildDescriptorHeap(ID3D12Device* device, UINT matIndex, UINT cbvIndex, UINT srvIndex) override;
	
	void UpdateDepthCamera(ID3D12GraphicsCommandList* cmdList, LightConstants& lightCnst);
	void PreRender(ID3D12GraphicsCommandList* cmdList, GameScene* scene);
	void RenderPipelines(ID3D12GraphicsCommandList* cmdList, int idx);

	void AppendTargetPipeline(Layer layer, Pipeline* pso);
	void SetShadowMapSRV(ID3D12GraphicsCommandList* cmdList, UINT srvIndex);

	void SetSunRange(float range) { mSunRange.push_back(range); }
	void SetCenter(const XMFLOAT3& center) { mCenter.push_back(center); }

	void UpdateSplitFrustum(const Camera* mainCamera);

	XMFLOAT4X4 GetShadowTransform(int idx) const;

private:
	void BuildDescriptorViews(ID3D12Device* device);

private:
	UINT mMapWidth;
	UINT mMapHeight;

	std::vector<XMFLOAT3> mCenter;
	std::vector<float> mZSplits;
	std::vector<float> mSunRange;

	UINT mMapCount;

	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRect;

	ComPtr<ID3D12DescriptorHeap> mDsvDescriptorHeap;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> mDsvCPUDescriptorHandles;

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> mCbvSrvCPUDescriptorHandles;

	std::vector<ComPtr<ID3D12Resource>> mShadowMaps;

	std::vector<std::unique_ptr<Camera>> mDepthCamera;
	std::map<Layer, Pipeline*> mShadowTargetPSOs;

	const XMFLOAT4X4 mToTexture =
	{
		0.5f,  0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f
	};

	const int OrthographicPlaneWidth = 1024;
	const int OrthographicPlaneHeight = 1024;

	ComPtr<ID3D12PipelineState> mTerrainPSO;
};