#include "stdafx.h"
#include "shadowMapRenderer.h"
#include "inGameScene.h"

ShadowMapRenderer::ShadowMapRenderer(ID3D12Device* device, UINT width, UINT height, UINT lightCount, const Camera* mainCamera, XMFLOAT3 shadowDirection)
	: mMapWidth(width), mMapHeight(height), mMapCount(lightCount)
{
	mViewPort = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };	
	
	for (int i = 0; i < (int)mMapCount; i++)
		mDepthCamera.push_back(std::make_unique<Camera>());

	mDsvCPUDescriptorHandles.resize(mMapCount, {});
	mCbvSrvCPUDescriptorHandles.resize(mMapCount, {});
	mCenter.resize(mMapCount, {});
	mSunRange.resize(mMapCount, {});
	mZSplits.resize(mMapCount + 1);

	mZSplits[0] = mainCamera->GetNearZ();
	mZSplits[mMapCount] = 1000;
	for (UINT i = 1; i < mMapCount; ++i)
	{
		float index = (i / (float)mMapCount);
		float uniformSplit = mZSplits[0] + (mZSplits[mMapCount] - mZSplits[0]) * index;
		float logarithmSplit = mZSplits[0] * std::powf((mZSplits[mMapCount] / mZSplits[0]), index);
		mZSplits[i] = std::lerp(logarithmSplit, uniformSplit, 0.3f);
		//mZSplits[i] = uniformSplit;
		//mZSplits[i] = logarithmSplit;
	}

	mShadowDirection = shadowDirection;
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

void ShadowMapRenderer::BuildPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, Shader* shader)
{
	auto shadowMapShader = std::make_unique<ShadowShader>(L"Shaders\\shadow.hlsl");
	auto shadowMapTerrainShader = std::make_unique<ShadowTerrainShader>(L"Shaders\\shadowTerrain.hlsl");
	auto shadowMapInstancingShader = std::make_unique<ShadowShader>(L"Shaders\\shadowInstancing.hlsl");

	mRasterizerDesc.DepthBias = 20000;
	mRasterizerDesc.DepthBiasClamp = 0.0f;
	mRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	mBackBufferFormat = DXGI_FORMAT_R32_FLOAT;
	mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	auto layout = shadowMapShader->GetInputLayout();

	psoDesc.pRootSignature = rootSig;
	psoDesc.InputLayout = {
		layout.data(),
		(UINT)layout.size()
	};
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(shadowMapShader->GetVS()->GetBufferPointer()),
		shadowMapShader->GetVS()->GetBufferSize()
	};
	if (shadowMapShader->GetGS() != nullptr)
	{
		psoDesc.GS = {
			reinterpret_cast<BYTE*>(shadowMapShader->GetGS()->GetBufferPointer()),
			shadowMapShader->GetGS()->GetBufferSize()
		};
	}
	if (shadowMapShader->GetDS() != nullptr)
	{
		psoDesc.DS = {
			reinterpret_cast<BYTE*>(shadowMapShader->GetDS()->GetBufferPointer()),
			shadowMapShader->GetDS()->GetBufferSize()
		};
	}
	if (shadowMapShader->GetHS() != nullptr)
	{
		psoDesc.HS = {
			reinterpret_cast<BYTE*>(shadowMapShader->GetHS()->GetBufferPointer()),
			shadowMapShader->GetHS()->GetBufferSize()
		};
	}

	psoDesc.RasterizerState = mRasterizerDesc;
	psoDesc.BlendState = mBlendDesc;
	psoDesc.DepthStencilState = mDepthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = mPrimitive;
	psoDesc.NumRenderTargets = 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	psoDesc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSO[0])));

	if (mIsWiredFrame) {
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		ThrowIfFailed(device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(&mPSO[1])));
	}

	auto Instancinglayout = shadowMapInstancingShader->GetInputLayout();

	psoDesc.pRootSignature = rootSig;
	psoDesc.InputLayout = {
		Instancinglayout.data(),
		(UINT)Instancinglayout.size()
	};
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(shadowMapInstancingShader->GetVS()->GetBufferPointer()),
		shadowMapInstancingShader->GetVS()->GetBufferSize()
	};
	if (shadowMapInstancingShader->GetGS() != nullptr)
	{
		psoDesc.GS = {
			reinterpret_cast<BYTE*>(shadowMapInstancingShader->GetGS()->GetBufferPointer()),
			shadowMapInstancingShader->GetGS()->GetBufferSize()
		};
	}
	if (shadowMapInstancingShader->GetDS() != nullptr)
	{
		psoDesc.DS = {
			reinterpret_cast<BYTE*>(shadowMapInstancingShader->GetDS()->GetBufferPointer()),
			shadowMapInstancingShader->GetDS()->GetBufferSize()
		};
	}
	if (shadowMapInstancingShader->GetHS() != nullptr)
	{
		psoDesc.HS = {
			reinterpret_cast<BYTE*>(shadowMapInstancingShader->GetHS()->GetBufferPointer()),
			shadowMapInstancingShader->GetHS()->GetBufferSize()
		};
	}

	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mInstancingPSO)));
	
	auto TerrainLayout = shadowMapTerrainShader->GetInputLayout();

	psoDesc.InputLayout = {
		TerrainLayout.data(),
		(UINT)TerrainLayout.size()
	};
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(shadowMapTerrainShader->GetVS()->GetBufferPointer()),
		shadowMapTerrainShader->GetVS()->GetBufferSize()
	};
	if (shadowMapTerrainShader->GetGS() != nullptr)
	{
		psoDesc.GS = {
			reinterpret_cast<BYTE*>(shadowMapTerrainShader->GetGS()->GetBufferPointer()),
			shadowMapTerrainShader->GetGS()->GetBufferSize()
		};
	}
	if (shadowMapTerrainShader->GetDS() != nullptr)
	{
		psoDesc.DS = {
			reinterpret_cast<BYTE*>(shadowMapTerrainShader->GetDS()->GetBufferPointer()),
			shadowMapTerrainShader->GetDS()->GetBufferSize()
		};
	}
	if (shadowMapTerrainShader->GetHS() != nullptr)
	{
		psoDesc.HS = {
			reinterpret_cast<BYTE*>(shadowMapTerrainShader->GetHS()->GetBufferPointer()),
			shadowMapTerrainShader->GetHS()->GetBufferSize()
		};
	}

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mTerrainPSO)));
}

void ShadowMapRenderer::UpdateSplitFrustum(const Camera* mainCamera)
{
	auto invView = mainCamera->GetInverseView();
	float vFov = tanf(mainCamera->GetFov().y / 2);
	float hFov = vFov * mainCamera->GetAspect();

	for (UINT i = 0; i < mMapCount; ++i)
	{
		float xn = mZSplits[i] * hFov;
		float xf = mZSplits[i + 1] * hFov;
		float yn = mZSplits[i] * vFov;
		float yf = mZSplits[i + 1] * vFov;

		XMFLOAT3 frustumCorners[8] =
		{
			//near Face
			{xn, yn, mZSplits[i]},
			{-xn, yn, mZSplits[i]},
			{xn, -yn ,mZSplits[i]},
			{-xn, -yn, mZSplits[i]},
			//far Face
			{xf, yf, mZSplits[i + 1]},
			{-xf, yf, mZSplits[i + 1]},
			{xf, -yf, mZSplits[i + 1]},
			{-xf, -yf, mZSplits[i + 1]}
		};

		XMFLOAT3 centerPos = { 0.0f, 0.0f, 0.0f };

		for (int j = 0; j < 8; ++j)
		{
			frustumCorners[j] = Vector3::Transform(frustumCorners[j], XMLoadFloat4x4(&invView));
			centerPos = Vector3::Add(centerPos, frustumCorners[j]);
		}
 		centerPos = Vector3::Divide(8, centerPos);

		mCenter[i] = centerPos;

		float sunRange = 0.0f;
		for (int j = 0; j < 8; ++j)
		{
			float distance = Vector3::Length(Vector3::Subtract(frustumCorners[j], centerPos));
			sunRange = std::max(sunRange, distance);
		}

		mSunRange[i] = sunRange;
	}
}

XMFLOAT4X4 ShadowMapRenderer::GetShadowTransform(int idx) const
{
	XMMATRIX view = XMLoadFloat4x4(&mDepthCamera[idx]->GetView());
	XMMATRIX proj = XMLoadFloat4x4(&mDepthCamera[idx]->GetProj());
	XMMATRIX T = XMLoadFloat4x4(&mToTexture);	
	
	XMFLOAT4X4 shadowT;
	XMStoreFloat4x4(&shadowT, view * proj * T);
	return shadowT;
}

void ShadowMapRenderer::BuildDescriptorHeap(ID3D12Device* device, UINT matIndex, UINT cbvIndex, UINT srvIndex)
{
	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			mMapCount,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mCbvSrvDescriptorHeap)));

	mRootParamMatIndex = matIndex;
	mRootParamCBVIndex = cbvIndex;
	mRootParamSRVIndex = srvIndex;

	BuildCBV(device);
	BuildSRV(device);

	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(mMapCount,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
		IID_PPV_ARGS(&mDsvDescriptorHeap)));

	BuildDescriptorViews(device);
}

void ShadowMapRenderer::BuildDescriptorViews(ID3D12Device* device)
{
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCPUHandle = mCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < mMapCount; ++i)
	{
		ComPtr<ID3D12Resource> shadowMap;
		shadowMap = CreateTexture2DResource(
			device, mMapWidth, mMapHeight, 1, 1,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue);

		mCbvSrvCPUDescriptorHandles[i] = cbvSrvCPUHandle;
		cbvSrvCPUHandle.ptr += gCbvSrvUavDescriptorSize;

		device->CreateShaderResourceView(shadowMap.Get(), &srvDesc, mCbvSrvCPUDescriptorHandles[i]);

		mDsvCPUDescriptorHandles[i] = dsvCPUHandle;
		dsvCPUHandle.ptr += gDsvDescriptorSize;

		device->CreateDepthStencilView(shadowMap.Get(), &dsvDesc, mDsvCPUDescriptorHandles[i]);

		mShadowMaps.push_back(shadowMap);
	}
}

void ShadowMapRenderer::UpdateDepthCamera(ID3D12GraphicsCommandList* cmdList)
{
	for (UINT i = 0; i < mMapCount; i++)
	{
		XMFLOAT3 look = Vector3::Normalize(mShadowDirection);
		XMFLOAT3 position = Vector3::MultiplyAdd(mSunRange[i], look, mCenter[i]);

		mDepthCamera[i]->LookAt(position, mCenter[i], XMFLOAT3(0.0f, 1.0f, 0.0f));
		
		mDepthCamera[i]->SetOrthographicLens(mCenter[i], mSunRange[i]);
	}

	for (UINT i = 1; i < mMapCount + 1; ++i)
	{
		cmdList->SetGraphicsRoot32BitConstants(8, 1, &mZSplits[i], i - 1);
	}
}

void ShadowMapRenderer::PreRender(ID3D12GraphicsCommandList* cmdList, InGameScene* scene)
{
	cmdList->RSSetViewports(1, &mViewPort);
	cmdList->RSSetScissorRects(1, &mScissorRect);
	cmdList->SetPipelineState(mPSO[0].Get());

	FLOAT clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	for (int i = 0; i < (int)mMapCount; i++)
	{
		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mShadowMaps[i].Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));

		cmdList->ClearDepthStencilView(mDsvCPUDescriptorHandles[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		cmdList->OMSetRenderTargets(0, nullptr, false, &mDsvCPUDescriptorHandles[i]);

		scene->UpdateCameraConstant(i + 1, mDepthCamera[i].get());

		Camera* cam[1] = { mDepthCamera[i].get() };
		scene->UpdateInstancingPipelines(cam, 1);

		scene->SetGraphicsCBV(cmdList, i + 1);
		RenderPipelines(cmdList, i);

		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mShadowMaps[i].Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_GENERIC_READ));
	}
}

void ShadowMapRenderer::RenderPipelines(ID3D12GraphicsCommandList* cmdList, int idx)
{
	for (const auto& [layer, pso] : mShadowTargetPSOs)
	{
		if (layer == Layer::Terrain)
		{
			cmdList->SetPipelineState(mTerrainPSO.Get());

			//pso->SetAndDraw(cmdList, mDepthCamera[idx]->GetWorldFrustum(), false, false, false);
			pso->SetAndDraw(cmdList, false, false);
			cmdList->SetPipelineState(mPSO[0].Get());
		}
		else if (layer == Layer::Instancing || layer == Layer::Transparent)
		{
			cmdList->SetPipelineState(mInstancingPSO.Get());

			pso->SetAndDraw(cmdList, false, false);
			cmdList->SetPipelineState(mPSO[0].Get());
		}
		else
		{
			//pso->SetAndDraw(cmdList, mDepthCamera[idx]->GetWorldFrustum(), true, false, false);
			pso->SetAndDraw(cmdList, false, false);
		}
	}
}

void ShadowMapRenderer::AppendTargetPipeline(Layer layer, Pipeline* pso)
{
	if(pso != nullptr)
		mShadowTargetPSOs.insert(std::make_pair(layer, pso));
}

void ShadowMapRenderer::SetShadowMapSRV(ID3D12GraphicsCommandList* cmdList, UINT srvIndex)
{
	ID3D12DescriptorHeap* descHeaps[] = { mCbvSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
	auto gpuHandle = mCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cmdList->SetGraphicsRootDescriptorTable(srvIndex, gpuHandle);
}