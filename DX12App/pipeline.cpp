#include "stdafx.h"
#include "pipeline.h"

Pipeline::Pipeline()
{
}

Pipeline::~Pipeline()
{
}

void Pipeline::BuildPipeline(
	ID3D12Device* device,
	ID3D12RootSignature* rootSig,
	Shader* shader)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	auto layout = shader->GetInputLayout();

	psoDesc.pRootSignature = rootSig;
	psoDesc.InputLayout = {
		layout.data(),
		(UINT)layout.size()
	};
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(shader->GetVS()->GetBufferPointer()),
		shader->GetVS()->GetBufferSize()
	};
	if (shader->GetGS() != nullptr)
	{
		psoDesc.GS = {
			reinterpret_cast<BYTE*>(shader->GetGS()->GetBufferPointer()),
			shader->GetGS()->GetBufferSize()
		};
	}
	if (shader->GetDS() != nullptr)
	{
		psoDesc.DS = {
			reinterpret_cast<BYTE*>(shader->GetDS()->GetBufferPointer()),
			shader->GetDS()->GetBufferSize()
		};
	}
	if (shader->GetHS() != nullptr)
	{
		psoDesc.HS = {
			reinterpret_cast<BYTE*>(shader->GetHS()->GetBufferPointer()),
			shader->GetHS()->GetBufferSize()
		};
	}
	if (shader->GetPS() != nullptr) 
	{
		psoDesc.PS = {
			reinterpret_cast<BYTE*>(shader->GetPS()->GetBufferPointer()),
			shader->GetPS()->GetBufferSize()
		};
	}

	mRasterizerDesc.AntialiasedLineEnable = mMsaaEnable;
	psoDesc.RasterizerState = mRasterizerDesc;

	psoDesc.SampleDesc.Count = mMsaaEnable ? 4 : 1;
	psoDesc.SampleDesc.Quality = mMsaaEnable ? mMsaa4xQualityLevels - 1 : 0;

	psoDesc.BlendState = mBlendDesc;
	psoDesc.DepthStencilState = mDepthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = mPrimitive;
	psoDesc.NumRenderTargets = 2;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.RTVFormats[1] = mVelocityMapFormat;
	psoDesc.DSVFormat = mDepthStencilFormat;

	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSO[0])));

	if (mIsWiredFrame) {
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		ThrowIfFailed(device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(&mPSO[1])));
	}

	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSO[2])));
}

void Pipeline::BuildConstantBuffer(ID3D12Device* device)
{
	if (mRenderObjects.size() > 0)
	{
		mObjectCB = std::make_unique<ConstantBuffer<ObjectConstants>>(device, (UINT)mRenderObjects.size());

		UINT matCount = 0;
		for (const auto& obj : mRenderObjects) matCount += obj->GetMeshCount();
		mMaterialCB = std::make_unique<ConstantBuffer<MaterialConstants>>(device, matCount);
	}
}

void Pipeline::BuildDescriptorHeap(ID3D12Device* device, UINT matIndex, UINT cbvIndex, UINT srvIndex)
{
	mRootParamMatIndex = matIndex;
	mRootParamCBVIndex = cbvIndex;
	mRootParamSRVIndex = srvIndex;

	UINT numDescriptors = (UINT)mRenderObjects.size();
	if (numDescriptors <= 0) return;

	for (const auto& obj : mRenderObjects)
	{
		numDescriptors += obj->GetTextureCount();
	}

	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			numDescriptors,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mCbvSrvDescriptorHeap)));

	BuildCBV(device);
	BuildSRV(device);
}

void Pipeline::BuildCBV(ID3D12Device* device)
{
	if (mObjectCB == nullptr) return;

	UINT stride = mObjectCB->GetByteSize();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = mObjectCB->GetGPUVirtualAddress(0);
	cbvDesc.SizeInBytes = stride;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	for (const auto& obj : mRenderObjects)
	{
		device->CreateConstantBufferView(&cbvDesc, cpuHandle);		
		obj->SetCBVAddress(gpuHandle);

		cbvDesc.BufferLocation += stride;
		cpuHandle.ptr += gCbvSrvUavDescriptorSize;
		gpuHandle.ptr += gCbvSrvUavDescriptorSize;
	}
}

void Pipeline::BuildSRV(ID3D12Device* device)
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	cpuHandle.ptr += mRenderObjects.size() * gCbvSrvUavDescriptorSize;
	gpuHandle.ptr += mRenderObjects.size() * gCbvSrvUavDescriptorSize;

	for (const auto& obj : mRenderObjects)
	{
		obj->SetSRVAddress(gpuHandle);
		obj->BuildSRV(device, cpuHandle);

		cpuHandle.ptr += obj->GetTextureCount() * gCbvSrvUavDescriptorSize;
		gpuHandle.ptr += obj->GetTextureCount() * gCbvSrvUavDescriptorSize;
	}
}

void Pipeline::AppendObject(const std::shared_ptr<GameObject>& obj)
{
	mRenderObjects.push_back(obj);
}

void Pipeline::DeleteObject(int idx)
{
	mRenderObjects.erase(mRenderObjects.begin() + idx);
}

std::vector<std::shared_ptr<GameObject>>::iterator Pipeline::DeleteObject(std::vector<std::shared_ptr<GameObject>>::iterator iter)
{
	return mRenderObjects.erase(iter);
}

void Pipeline::ResetPipeline(ID3D12Device* device)
{
	BuildConstantBuffer(device);
	BuildDescriptorHeap(device, mRootParamMatIndex, mRootParamCBVIndex, mRootParamSRVIndex);
}

void Pipeline::PreparePipeline(ID3D12GraphicsCommandList* cmdList, bool drawWiredFrame, bool setPipeline, bool msaaOff)
{
	if (mCbvSrvDescriptorHeap.Get() == nullptr)
		return;

	ID3D12DescriptorHeap* descHeaps[] = { mCbvSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
	cmdList->OMSetStencilRef(mStencilRef);

	if (setPipeline) {
		if (mIsWiredFrame && drawWiredFrame)
			cmdList->SetPipelineState(mPSO[1].Get());
		else if (msaaOff)
			cmdList->SetPipelineState(mPSO[2].Get());
		else
			cmdList->SetPipelineState(mPSO[0].Get());
	}
}

void Pipeline::SetAndDraw(ID3D12GraphicsCommandList* cmdList, bool drawWiredFrame, bool setPipeline, bool msaaOff, DrawType type)
{	
	PreparePipeline(cmdList, drawWiredFrame, setPipeline, msaaOff);
	Draw(cmdList, false, type);
}

void Pipeline::Draw(ID3D12GraphicsCommandList* cmdList, bool isSO, DrawType type)
{
	UINT matOffset = 0;
	for (int i = 0; i < mRenderObjects.size(); i++)
	{
		if (mMaterialCB)
		{
			mRenderObjects[i]->Draw(
				cmdList,
				mRootParamMatIndex,
				mRootParamCBVIndex,
				mRootParamSRVIndex,
				mMaterialCB->GetGPUVirtualAddress(matOffset),
				mMaterialCB->GetByteSize(), isSO);

			matOffset += mRenderObjects[i]->GetMeshCount();
		}
		else
		{
			mRenderObjects[i]->Draw(
				cmdList,
				mRootParamMatIndex,
				mRootParamCBVIndex,
				mRootParamSRVIndex,
				NULL,
				NULL, isSO);

		}
	}
}

void Pipeline::SetCullClockwise()
{
	mRasterizerDesc.FrontCounterClockwise = true;
}

void Pipeline::SetAlphaBlending()
{
	D3D12_RENDER_TARGET_BLEND_DESC rtBlend{};
	rtBlend.BlendEnable = TRUE;
	rtBlend.LogicOpEnable = FALSE;
	rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	mBlendDesc.RenderTarget[0] = rtBlend;
}

void Pipeline::SetStencilOp(
	UINT stencilRef, D3D12_DEPTH_WRITE_MASK depthWriteMask,
	D3D12_STENCIL_OP stencilFail, D3D12_STENCIL_OP stencilDepthFail, 
	D3D12_STENCIL_OP stencilPass, D3D12_COMPARISON_FUNC stencilFunc, UINT8 rtWriteMask)
{
	D3D12_DEPTH_STENCILOP_DESC depthStencilDesc{};
	depthStencilDesc.StencilFailOp = stencilFail;
	depthStencilDesc.StencilDepthFailOp = stencilDepthFail;
	depthStencilDesc.StencilPassOp = stencilPass;
	depthStencilDesc.StencilFunc = stencilFunc;
	
	mDepthStencilDesc.StencilEnable = TRUE;
	mDepthStencilDesc.DepthWriteMask = depthWriteMask;
	mDepthStencilDesc.FrontFace = depthStencilDesc;
	mDepthStencilDesc.BackFace = depthStencilDesc;
	mBlendDesc.RenderTarget[0].RenderTargetWriteMask = rtWriteMask;

	mStencilRef = stencilRef;
}

void Pipeline::SortMeshes()
{
	for (int i = 0; i < mRenderObjects.size(); ++i)
	{
		mRenderObjects[i]->SortMeshes();
	}
}

void Pipeline::Update(float elapsed, float updateRate, Camera* camera)
{
	for (const auto& obj : mRenderObjects)
		obj->Update(elapsed, updateRate);
}

void Pipeline::UpdateConstants(Camera* camera, DrawType type, bool culling)
{
	UINT matOffset = 0;
	for (int i = 0; i < mRenderObjects.size(); i++) {
		mObjectCB->CopyData(i, mRenderObjects[i]->GetObjectConstants());

		if (mMaterialCB)
		{
			mRenderObjects[i]->UpdateMatConstants(mMaterialCB.get(), matOffset);
			matOffset += mRenderObjects[i]->GetMeshCount();
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//
SkyboxPipeline::SkyboxPipeline(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	: Pipeline()
{
	auto boxMesh = std::make_shared<BoxMesh>(device, cmdList, 20.0f, 20.0f, 20.0f);
	boxMesh->SetSrvIndex(0);

	auto skyboxObj = std::make_shared<GameObject>();
	skyboxObj->SetMesh(boxMesh);
	skyboxObj->LoadTexture(device, cmdList, L"Resources\\skyboxarray_night.dds", D3D12_SRV_DIMENSION_TEXTURE2DARRAY);
	mRenderObjects.push_back(skyboxObj);
}

SkyboxPipeline::~SkyboxPipeline()
{
}

void SkyboxPipeline::BuildPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, Shader* shader)
{
	auto skyboxShader = std::make_shared<DefaultShader>(L"Shaders\\skybox_VS.cso", L"Shaders\\skybox_PS.cso");
		
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	auto layout = skyboxShader->GetInputLayout();
	psoDesc.pRootSignature = rootSig;
	psoDesc.InputLayout = {
		layout.data(),
		(UINT)layout.size()
	};
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(skyboxShader->GetVS()->GetBufferPointer()),
		skyboxShader->GetVS()->GetBufferSize()
	};
	psoDesc.PS = {
		reinterpret_cast<BYTE*>(skyboxShader->GetPS()->GetBufferPointer()),
		skyboxShader->GetPS()->GetBufferSize()
	};

	mRasterizerDesc.AntialiasedLineEnable = mMsaaEnable;
	psoDesc.RasterizerState = mRasterizerDesc;

	psoDesc.SampleDesc.Count = mMsaaEnable ? 4 : 1;
	psoDesc.SampleDesc.Quality = mMsaaEnable ? mMsaa4xQualityLevels - 1 : 0;

	psoDesc.BlendState = mBlendDesc;
	psoDesc.DepthStencilState = mDepthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = mPrimitive;
	psoDesc.NumRenderTargets = 2;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.RTVFormats[1] = mVelocityMapFormat;
	psoDesc.DSVFormat = mDepthStencilFormat;

	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSO[0])));

	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSO[2])));
}


/////////////////////////////////////////////////////////////////////////
//
StreamOutputPipeline::StreamOutputPipeline(UINT objectMaxCount)
	: Pipeline(), mStreamOutputDesc({}), mObjectMaxCount(objectMaxCount)
{
}

StreamOutputPipeline::~StreamOutputPipeline()
{
}

void StreamOutputPipeline::BuildPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, Shader* shader)
{
	auto renderShader = std::make_unique<BillboardShader>(L"Shaders\\billboard_VSRender.cso", L"Shaders\\billboard_GSRender.cso", L"Shaders\\billboard_PSRender.cso", true);
	auto soShader = std::make_unique<BillboardShader>(L"Shaders\\billboard_VSStreamOutput.cso", L"Shaders\\billboard_GSStreamOutput.cso", L"", true);

	SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	SetAlphaBlending();
	CreateStreamOutputDesc();

	Pipeline::BuildPipeline(device, rootSig, renderShader.get());
	BuildSOPipeline(device, rootSig, soShader.get());
}

void StreamOutputPipeline::SetAndDraw(
	ID3D12GraphicsCommandList* cmdList, 
	bool drawWiredFrame, 
	bool setPipeline,
	bool msaaOff, DrawType type)
{
	ID3D12DescriptorHeap* descHeaps[] = { mCbvSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
	cmdList->OMSetStencilRef(mStencilRef);
	
	cmdList->SetPipelineState(mPSO[1].Get());
	Pipeline::Draw(cmdList, true);

	cmdList->SetPipelineState(mPSO[0].Get()); // Draw
	Pipeline::Draw(cmdList, false);
}

void StreamOutputPipeline::BuildDescriptorHeap(ID3D12Device* device, UINT matIndex, UINT cbvIndex, UINT srvIndex)
{
	mRootParamMatIndex = matIndex;
	mRootParamCBVIndex = cbvIndex;
	mRootParamSRVIndex = srvIndex;

	UINT numDescriptors = mObjectMaxCount * 2;

	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			numDescriptors,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mCbvSrvDescriptorHeap)));

	mCbvSrvDescriptorHeap->SetName(L"SO Descriptor Heap");

	BuildCBV(device);
	BuildSRV(device);
}

void StreamOutputPipeline::BuildConstantBuffer(ID3D12Device* device)
{
	mObjectCB = std::make_unique<ConstantBuffer<ObjectConstants>>(device, mObjectMaxCount);
}

void StreamOutputPipeline::BuildSOPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, Shader* shader)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	auto layout = shader->GetInputLayout();

	psoDesc.pRootSignature = rootSig;
	psoDesc.InputLayout = {
		layout.data(),
		(UINT)layout.size()
	};
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(shader->GetVS()->GetBufferPointer()),
		shader->GetVS()->GetBufferSize()
	};
	psoDesc.GS = {
		reinterpret_cast<BYTE*>(shader->GetGS()->GetBufferPointer()),
		shader->GetGS()->GetBufferSize()
	};
	psoDesc.PS = { NULL, 0 };
	psoDesc.RasterizerState = mRasterizerDesc;
	psoDesc.BlendState = mBlendDesc;
	psoDesc.DepthStencilState = mDepthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = mPrimitive;
	psoDesc.NumRenderTargets = 0;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc.DSVFormat = mDepthStencilFormat;
	psoDesc.StreamOutput = mStreamOutputDesc;
	psoDesc.SampleDesc.Count = 1;
	//psoDesc.SampleDesc.Quality = mMsaaEnable ? mMsaa4xQualityLevels - 1 : 0;

	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSO[1])));
}

void StreamOutputPipeline::AppendObject(ID3D12Device* device, const std::shared_ptr<GameObject>& obj)
{
	obj->SetCBVAddress(mConstantBufferGPUHandles[mRenderObjects.size()]);
	obj->SetSRVAddress(mShaderResourceGPUHandles[mRenderObjects.size()]);
	
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	cpuHandle.ptr += mObjectMaxCount * gCbvSrvUavDescriptorSize;
	cpuHandle.ptr += mRenderObjects.size() * gCbvSrvUavDescriptorSize;
	
	obj->BuildSRV(device, cpuHandle);
	
	mRenderObjects.push_back(obj);
}

void StreamOutputPipeline::CreateStreamOutputDesc()
{
	mSODeclarations.push_back({ 0, "POSITION",  0, 0, 3, 0 });
	mSODeclarations.push_back({ 0, "SIZE",      0, 0, 2, 0 });
	mSODeclarations.push_back({ 0, "COLOR",     0, 0, 4, 0 });
	mSODeclarations.push_back({ 0, "VELOCITY",  0, 0, 3, 0 });
	mSODeclarations.push_back({ 0, "LIFETIME",  0, 0, 2, 0 });
	mSODeclarations.push_back({ 0, "TYPE",	    0, 0, 1, 0 });
	
	mStrides.push_back(sizeof(BillboardVertex));

	ZeroMemory(&mStreamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));
	mStreamOutputDesc.NumEntries = (UINT)mSODeclarations.size();
	mStreamOutputDesc.NumStrides = (UINT)mStrides.size();
	mStreamOutputDesc.pBufferStrides = mStrides.data();
	mStreamOutputDesc.pSODeclaration = mSODeclarations.data();
	mStreamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;
}

void StreamOutputPipeline::BuildCBV(ID3D12Device* device)
{
	if (mObjectCB == nullptr) return;

	UINT stride = mObjectCB->GetByteSize();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = mObjectCB->GetGPUVirtualAddress(0);
	cbvDesc.SizeInBytes = stride;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < mObjectMaxCount; ++i)
	{
		device->CreateConstantBufferView(&cbvDesc, cpuHandle);
		
		mConstantBufferGPUHandles.push_back(gpuHandle);

		cbvDesc.BufferLocation += stride;
		cpuHandle.ptr += gCbvSrvUavDescriptorSize;
		gpuHandle.ptr += gCbvSrvUavDescriptorSize;
	}
}

void StreamOutputPipeline::BuildSRV(ID3D12Device* device)
{
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	gpuHandle.ptr += mObjectMaxCount * gCbvSrvUavDescriptorSize;

	for (UINT i = 0; i < mObjectMaxCount; ++i)
	{
		mShaderResourceGPUHandles.push_back(gpuHandle);

		gpuHandle.ptr += gCbvSrvUavDescriptorSize;
	}
}

/////////////////////////////////////////////////////////////////////////
//
ComputePipeline::ComputePipeline()
{
}

ComputePipeline::~ComputePipeline()
{
}

void ComputePipeline::BuildPipeline(
	ID3D12Device* device, 
	ID3D12RootSignature* rootSig, 
	ComputeShader* shader, bool init)
{
	mComputeRootSig = rootSig;

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSig;
	psoDesc.CS =
	{
		reinterpret_cast<BYTE*>(shader->GetCS()->GetBufferPointer()),
		shader->GetCS()->GetBufferSize()
	};
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.NodeMask = 0;

	mPSOs.push_back({});
	ThrowIfFailed(device->CreateComputePipelineState(
		&psoDesc, IID_PPV_ARGS(&mPSOs.back())));
}

void ComputePipeline::Dispatch(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetPipelineState(mPSOs[0].Get());

	UINT numGroupX = (UINT)(ceilf(gFrameWidth / 32.0f));
	UINT numGroupY = (UINT)(ceilf(gFrameHeight / 30.0f));

	cmdList->Dispatch(numGroupX, numGroupY, 1);
}

/////////////////////////////////////////////////////////////////////////
//
InstancingPipeline::InstancingPipeline()
{

}

InstancingPipeline::~InstancingPipeline()
{

}

void InstancingPipeline::Draw(ID3D12GraphicsCommandList* cmdList, bool isSO, DrawType type)
{
	if (mRenderObjects.size() <= 0) return;

	UINT matOffset = 0;
	UINT instancingOffset = 0;

	cmdList->SetGraphicsRootShaderResourceView(9, mInstancingSB[(int)type]->GetGPUVirtualAddress(0));

	int instance = 0;
	for (auto [_, count] : mInstancingCount)
	{
		if (count > 0) instance += count;
	}

	//OutputDebugStringA(std::string("Drawing Object Count : " + std::to_string(instance) + "\n\n").c_str());
	for (int i = 0; i < mRenderObjects.size(); i++)
	{
		if (mRenderObjects[i]->GetMeshCount() > 0 && mInstancingCount[mRenderObjects[i]->GetName()] > 0)
		{
			cmdList->SetGraphicsRoot32BitConstants(8, 1, &instancingOffset, 3);

			mRenderObjects[i]->DrawInstanced(
				cmdList,
				mRootParamMatIndex,
				mRootParamSBIndex,
				mRootParamSRVIndex,
				mMaterialCB->GetGPUVirtualAddress(matOffset),
				mMaterialCB->GetByteSize(), mInstancingCount[mRenderObjects[i]->GetName()], isSO);

			matOffset += mRenderObjects[i]->GetMeshCount();
			instancingOffset += mInstancingCount[mRenderObjects[i]->GetName()];
			//OutputDebugStringA(std::string(mRenderObjects[i]->GetName() + "'s instancingOffset : " + std::to_string(instancingOffset) + "\n").c_str());
		}
	}
	//OutputDebugStringA("\n");
}

void InstancingPipeline::BuildConstantBuffer(ID3D12Device* device)
{
	if (mRenderObjects.size() > 0)
	{
		for(int i = 0; i < 5; ++i)
			mInstancingSB[i] = std::make_unique<StructuredBuffer<InstancingInfo>>(device, (UINT)mRenderObjects.size());

		UINT matCount = 0;
		for (const auto& obj : mRenderObjects) matCount += obj->GetMeshCount();
		mMaterialCB = std::make_unique<ConstantBuffer<MaterialConstants>>(device, matCount);
	}
}

void InstancingPipeline::UpdateConstants(Camera* camera, DrawType type, bool culling)
{
	UINT matOffset = 0;
	for (auto& [_, count] : mInstancingCount)
	{
		count = 0;
	}
	if (culling)
	{
		std::map<int, bool> collidedIndex;
		XMMATRIX invView = XMLoadFloat4x4(&camera->GetInverseView());
		auto viewFrustum = camera->GetViewFrustum();
		
		for (int i = 0; i < mRenderObjects.size(); i++)
		{
			XMMATRIX invWorld = XMLoadFloat4x4(&mRenderObjects[i]->GetInverseWorld());

			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			BoundingFrustum localFrustum;
			viewFrustum.Transform(localFrustum, viewToLocal);
			if (localFrustum.Contains(mRenderObjects[i]->GetBoundingBox()) != DirectX::DISJOINT)
			{
				collidedIndex[i] = true;
				mInstancingCount[mRenderObjects[i]->GetName()]++;
			}
			else
			{
				collidedIndex[i] = false;
			}
		}

		int currentIndex = 0;
		for (int i = 0; i < mRenderObjects.size(); ++i)
		{
			if (collidedIndex[i])
			{
				mInstancingSB[(int)type]->CopyData(currentIndex, mRenderObjects[i]->GetInstancingInfo());
				if (mRenderObjects[i]->GetMeshCount() > 0)
				{
					mRenderObjects[i]->UpdateMatConstants(mMaterialCB.get(), matOffset);
					matOffset += mRenderObjects[i]->GetMeshCount();
				}
				currentIndex++;
			}
		}
	}
	else
	{
		for (int i = 0; i < mRenderObjects.size(); i++) 
		{
			mInstancingCount[mRenderObjects[i]->GetName()]++;
		}

		for (int i = 0; i < mRenderObjects.size(); i++)
		{
			mInstancingSB[(int)type]->CopyData(i, mRenderObjects[i]->GetInstancingInfo());
			if (mRenderObjects[i]->GetMeshCount() > 0)
			{
				mRenderObjects[i]->UpdateMatConstants(mMaterialCB.get(), matOffset);
				matOffset += mRenderObjects[i]->GetMeshCount();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
//
MotionBlurPipeline::MotionBlurPipeline()
{
}

MotionBlurPipeline::~MotionBlurPipeline()
{
}

void MotionBlurPipeline::BuildPipeline(ID3D12Device* device, ID3D12RootSignature* rootSig, ComputeShader* shader, bool init)
{
	ComputePipeline::BuildPipeline(device, rootSig, shader);

	if (init)
	{
		CreateTextures(device);
		BuildDescriptorHeap(device);
	}
}

void MotionBlurPipeline::SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn)
{
	if (msaaOn)
		ResolveRTToMap(cmdList, buffer, mInputTexture[idx]->GetResource(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	else
		CopyRTToMap(cmdList, buffer, mInputTexture[idx]->GetResource());
}

void MotionBlurPipeline::CreateTextures(ID3D12Device* device)
{
	mInputTexture[0] = std::make_unique<Texture>();
	mInputTexture[0]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mInputTexture[0]->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON, nullptr);

	mInputTexture[1] = std::make_unique<Texture>();
	mInputTexture[1]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mInputTexture[1]->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON, nullptr);

	mOutputTexture = std::make_unique<Texture>();
	mOutputTexture->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mOutputTexture->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON, nullptr);
}

void MotionBlurPipeline::BuildDescriptorHeap(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			3,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mSrvUavDescriptorHeap)));

	BuildSRVAndUAV(device);
}

void MotionBlurPipeline::BuildSRVAndUAV(ID3D12Device* device)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	auto cpuHandle = mSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	device->CreateShaderResourceView(mInputTexture[0]->GetResource(), &srvDesc, cpuHandle);
	cpuHandle.ptr += gCbvSrvUavDescriptorSize;

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateShaderResourceView(mInputTexture[1]->GetResource(), &srvDesc, cpuHandle);
	cpuHandle.ptr += gCbvSrvUavDescriptorSize;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mOutputTexture->GetResource(), nullptr, &uavDesc, cpuHandle);
}

void MotionBlurPipeline::CopyRTToMap(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource* dest)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		source, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		dest, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	cmdList->CopyResource(dest, source);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		source, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void MotionBlurPipeline::ResolveRTToMap(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource* dest, DXGI_FORMAT format)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		source, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		dest, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_DEST));

	cmdList->ResolveSubresource(dest, 0, source, 0, format);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		dest, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		source, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void MotionBlurPipeline::CopyMapToRT(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtBuffer)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		rtBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST));

	cmdList->CopyResource(rtBuffer, mOutputTexture->GetResource());

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		rtBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void MotionBlurPipeline::CopyCurrentToPreviousBuffer(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mInputTexture[0]->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mInputTexture[1]->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));

	cmdList->CopyResource(mInputTexture[0]->GetResource(), mInputTexture[1]->GetResource());

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mInputTexture[0]->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mInputTexture[1]->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
}

void MotionBlurPipeline::Dispatch(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetComputeRootSignature(mComputeRootSig);
	ID3D12DescriptorHeap* descHeap[] = { mSrvUavDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeap), descHeap);

	cmdList->SetComputeRoot32BitConstant(2, gFrameWidth, 0);
	cmdList->SetComputeRoot32BitConstant(2, gFrameHeight, 1);

	auto gpuHandle = mSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cmdList->SetComputeRootDescriptorTable(0, gpuHandle);

	gpuHandle.ptr += 2 * gCbvSrvUavDescriptorSize;
	cmdList->SetComputeRootDescriptorTable(1, gpuHandle);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mOutputTexture->GetResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->SetPipelineState(mPSOs[0].Get());

	UINT numGroupX = (UINT)(ceilf(gFrameWidth / 32.0f));
	UINT numGroupY = (UINT)(ceilf(gFrameHeight / 30.0f));

	cmdList->Dispatch(numGroupX, numGroupY, 1);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mOutputTexture->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON));
}

BloomPipeline::BloomPipeline()
{
	// compute Gaussian kernel
	float sigma = 10.f;
	float sigmaRcp = 1.f / sigma;
	float twoSigmaSq = 2 * sigma * sigma;

	float sum = 0.f;
	for (size_t i = 0; i <= GAUSSIAN_RADIUS; ++i)
	{
		// we omit the normalization factor here for the discrete version and normalize using the sum afterwards
		mBlurCoefficients[i] = (1.f / sigma) * std::expf(-static_cast<float>(i * i) / twoSigmaSq);
		// we use each entry twice since we only compute one half of the curve
		sum += 2 * mBlurCoefficients[i];
	}
	// the center (index 0) has been counted twice, so we subtract it once
	sum -= mBlurCoefficients[0];

	// we normalize all entries using the sum so that the entire kernel gives us a sum of coefficients = 0
	float normalizationFactor = 1.f / sum;
	for (size_t i = 0; i <= GAUSSIAN_RADIUS; ++i)
	{
		mBlurCoefficients[i] *= normalizationFactor;
	}
}

BloomPipeline::~BloomPipeline()
{
}

void BloomPipeline::SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn)
{
	CopyRTToMap(cmdList, buffer, mInputTexture[0]->GetResource());
}

void BloomPipeline::Dispatch(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetComputeRootSignature(mComputeRootSig);
	ID3D12DescriptorHeap* descHeap[] = { mSrvUavDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeap), descHeap);

	// Input Texture와 ProcessingTexture[0]를 이용해 다운 샘플링
	float threshold = 0.5f;
	cmdList->SetComputeRoot32BitConstants(2, 1, &threshold, 0);

	auto gpuHandle = mSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cmdList->SetComputeRootDescriptorTable(0, gpuHandle);

	gpuHandle.ptr += 3 * gCbvSrvUavDescriptorSize;
	cmdList->SetComputeRootDescriptorTable(1, gpuHandle);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mProcessingTexture[0]->GetResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->SetPipelineState(mPSOs[0].Get());


	// 원본의 절반 해상도이므로 원본 / 32 * 2
	UINT numGroupX = (UINT)(ceilf(gFrameWidth / 64.0f));
	UINT numGroupY = (UINT)(ceilf(gFrameHeight / 64.0f));

	cmdList->Dispatch(numGroupX, numGroupY, 1);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mProcessingTexture[0]->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON));

	// ProcessingTexture 2개를 이용하여 2-Pass Gaussian Blur
	cmdList->SetPipelineState(mPSOs[1].Get());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle[2];
	D3D12_GPU_DESCRIPTOR_HANDLE uavHandle[2];

	gpuHandle.ptr -= 2 * gCbvSrvUavDescriptorSize;
	srvHandle[0] = gpuHandle;
	gpuHandle.ptr += gCbvSrvUavDescriptorSize;
	srvHandle[1] = gpuHandle;

	gpuHandle.ptr += gCbvSrvUavDescriptorSize;
	uavHandle[1] = gpuHandle;
	gpuHandle.ptr += gCbvSrvUavDescriptorSize;
	uavHandle[0] = gpuHandle;

	for (int i = 0; i < 2; ++i)
	{
		cmdList->SetComputeRootDescriptorTable(0, srvHandle[i]);
		cmdList->SetComputeRootDescriptorTable(1, uavHandle[i]);

		cmdList->SetComputeRoot32BitConstants(2, 4, mBlurCoefficients, 0);

		cmdList->SetComputeRoot32BitConstant(2, GAUSSIAN_RADIUS, 4);
		cmdList->SetComputeRoot32BitConstant(2, i, 5);

		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mProcessingTexture[i]->GetResource(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		cmdList->Dispatch(numGroupX, numGroupY, 1);

		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mProcessingTexture[i]->GetResource(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COMMON));
	}

	uavHandle[0].ptr += gCbvSrvUavDescriptorSize;

	numGroupX = (UINT)(ceilf(gFrameWidth / 32.0f));
	numGroupY = (UINT)(ceilf(gFrameHeight / 32.0f));

	cmdList->SetPipelineState(mPSOs[2].Get());

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mProcessingTexture[2]->GetResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->SetComputeRootDescriptorTable(0, mSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetComputeRootDescriptorTable(1, uavHandle[0]);

	float mergeCoefficient = 0.75;
	cmdList->SetComputeRoot32BitConstants(2, 1, &mergeCoefficient, 0);

	cmdList->Dispatch(numGroupX, numGroupY, 1);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mProcessingTexture[2]->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE));
}

void BloomPipeline::CreateTextures(ID3D12Device* device)
{
	mInputTexture[0] = std::make_unique<Texture>();
	mInputTexture[0]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mInputTexture[0]->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON, nullptr);


	for (int i = 0; i < 2; ++i)
	{
		mProcessingTexture[i] = std::make_unique<Texture>();
		mProcessingTexture[i]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
		mProcessingTexture[i]->CreateTexture(device, gFrameWidth / 2, gFrameHeight / 2,
			1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COMMON, nullptr);
	}

	mProcessingTexture[2] = std::make_unique<Texture>();
	mProcessingTexture[2]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mProcessingTexture[2]->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON, nullptr);
}

void BloomPipeline::BuildDescriptorHeap(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			6,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mSrvUavDescriptorHeap)));

	BuildSRVAndUAV(device);
}

void BloomPipeline::BuildSRVAndUAV(ID3D12Device* device)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	auto cpuHandle = mSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateShaderResourceView(mInputTexture[0]->GetResource(), &srvDesc, cpuHandle);
	cpuHandle.ptr += gCbvSrvUavDescriptorSize;


	for (int i = 0; i < 2; ++i)
	{
		device->CreateShaderResourceView(mProcessingTexture[i]->GetResource(), &srvDesc, cpuHandle);
		cpuHandle.ptr += gCbvSrvUavDescriptorSize;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	for (int i = 0; i < 3; ++i)
	{
		device->CreateUnorderedAccessView(mProcessingTexture[i]->GetResource(), nullptr, &uavDesc, cpuHandle);
		cpuHandle.ptr += gCbvSrvUavDescriptorSize;
	}
}

void BloomPipeline::CopyMapToRT(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtBuffer)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		rtBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST));

	cmdList->CopyResource(rtBuffer, mProcessingTexture[2]->GetResource());

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		rtBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mProcessingTexture[2]->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
}

VolumetricScatteringPipeline::VolumetricScatteringPipeline()
{
}

VolumetricScatteringPipeline::~VolumetricScatteringPipeline()
{
}

void VolumetricScatteringPipeline::SetInput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* buffer, int idx, bool msaaOn)
{
	if (msaaOn)
		ResolveRTToMap(cmdList, buffer, mInputTexture[idx]->GetResource(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	else
		CopyRTToMap(cmdList, buffer, mInputTexture[idx]->GetResource());
}

void VolumetricScatteringPipeline::Dispatch(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetComputeRootSignature(mComputeRootSig);
	ID3D12DescriptorHeap* descHeap[] = { mSrvUavDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeap), descHeap);

	auto gpuHandle = mSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cmdList->SetComputeRootDescriptorTable(0, gpuHandle);

	gpuHandle.ptr += 2 * gCbvSrvUavDescriptorSize;
	cmdList->SetComputeRootDescriptorTable(1, gpuHandle);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mOutputTexture->GetResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->SetPipelineState(mPSOs[0].Get());

	UINT numGroupX = (UINT)(ceilf(gFrameWidth / 32.0f));
	UINT numGroupY = (UINT)(ceilf(gFrameHeight / 30.0f));

	cmdList->Dispatch(numGroupX, numGroupY, 1);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mOutputTexture->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON));
}

void VolumetricScatteringPipeline::CreateTextures(ID3D12Device* device)
{
	mInputTexture[0] = std::make_unique<Texture>();
	mInputTexture[0]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mInputTexture[0]->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON, nullptr);


	mInputTexture[1] = std::make_unique<Texture>();
	mInputTexture[1]->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mInputTexture[1]->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON, nullptr);

	mOutputTexture = std::make_unique<Texture>();
	mOutputTexture->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mOutputTexture->CreateTexture(device, gFrameWidth, gFrameHeight,
		1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON, nullptr);
}

void VolumetricScatteringPipeline::BuildDescriptorHeap(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			3,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mSrvUavDescriptorHeap)));

	BuildSRVAndUAV(device);
}

void VolumetricScatteringPipeline::BuildSRVAndUAV(ID3D12Device* device)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	auto cpuHandle = mSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateShaderResourceView(mInputTexture[0]->GetResource(), &srvDesc, cpuHandle);
	cpuHandle.ptr += gCbvSrvUavDescriptorSize;

	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	device->CreateShaderResourceView(mInputTexture[1]->GetResource(), &srvDesc, cpuHandle);
	cpuHandle.ptr += gCbvSrvUavDescriptorSize;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mOutputTexture->GetResource(), nullptr, &uavDesc, cpuHandle);
}

void VolumetricScatteringPipeline::ResolveRTToMap(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource* dest, DXGI_FORMAT format)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		source, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		dest, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_DEST));

	cmdList->ResolveSubresource(dest, 0, source, 0, format);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		dest, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		source, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}