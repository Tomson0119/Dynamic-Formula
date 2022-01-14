#include "stdafx.h"
#include "shadowMapRenderer.h"
#include "gameScene.h"

ShadowMapRenderer::ShadowMapRenderer(ID3D12Device* device, UINT width, UINT height, UINT lightCount)
	: mMapWidth(width), mMapHeight(height), mMapCount(lightCount)
{
	mViewPort = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };	
	
	for (int i = 0; i < (int)mMapCount; i++)
		mDepthCamera.push_back(std::make_unique<Camera>());

	mDsvCPUDescriptorHandles.resize(mMapCount, {});
	mCbvSrvCPUDescriptorHandles.resize(mMapCount, {});
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

void ShadowMapRenderer::BuildPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, Shader* shader)
{
	auto shadowMapShader = std::make_unique<ShadowShader>(L"Shaders\\shadow.hlsl");
	auto shadowMapTerrainShader = std::make_unique<ShadowTerrainShader>(L"Shaders\\shadowTerrain.hlsl");

	mRasterizerDesc.DepthBias = 100000;
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

	for (int i = 0; i < mMapCount; ++i)
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

void ShadowMapRenderer::UpdateDepthCamera(LightConstants& lightCnst)
{
	for (int i = 0; i < (int)mMapCount; i++)
	{
		XMFLOAT3 look = lightCnst.Lights[i].Direction;
		XMFLOAT3 position = Vector3::MultiplyAdd(mSunRange, look, mCenter);

		mDepthCamera[i]->LookAt(position, mCenter, XMFLOAT3(0.0f, 1.0f, 0.0f));

		switch (lightCnst.Lights[i].Type)
		{
		case DIRECTIONAL_LIGHT:
			mDepthCamera[i]->SetOrthographicLens(mCenter, mSunRange);
			break;

		case SPOT_LIGHT:
			mDepthCamera[i]->SetLens(Math::PI * 0.333f,
				(float)mMapWidth / (float)mMapHeight,
				1.0f, lightCnst.Lights[i].Range);
			break;

		case POINT_LIGHT:
			// need 6 shadow maps
			break;
		}

		mDepthCamera[i]->UpdateViewMatrix();
	}
}

void ShadowMapRenderer::PreRender(ID3D12GraphicsCommandList* cmdList, GameScene* scene)
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
		scene->SetCBV(cmdList, i + 1);
		RenderPipelines(cmdList);

		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mShadowMaps[i].Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_GENERIC_READ));
	}
}

void ShadowMapRenderer::RenderPipelines(ID3D12GraphicsCommandList* cmdList)
{
	for (const auto& [layer, pso] : mShadowTargetPSOs)
	{
		if (layer == Layer::Terrain)
		{
			cmdList->SetPipelineState(mTerrainPSO.Get());
		}

		pso->SetAndDraw(cmdList, false, false);

		if (layer == Layer::Terrain)
		{
			cmdList->SetPipelineState(mPSO[0].Get());
		}

	}
}

void ShadowMapRenderer::AppendTargetPipeline(Layer layer, Pipeline* pso)
{
	mShadowTargetPSOs.insert(std::make_pair(layer, pso));
}

void ShadowMapRenderer::SetShadowMapSRV(ID3D12GraphicsCommandList* cmdList, UINT srvIndex)
{
	ID3D12DescriptorHeap* descHeaps[] = { mCbvSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
	auto gpuHandle = mCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cmdList->SetGraphicsRootDescriptorTable(srvIndex, gpuHandle);
}