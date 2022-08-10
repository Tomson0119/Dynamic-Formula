#include "stdafx.h"
#include "inGameScene.h"
#include "shadowMapRenderer.h"
#include "NetLib/NetModule.h"

using namespace std;

InGameScene::InGameScene(HWND hwnd, NetModule* netPtr, bool msaaEnable, UINT msaaQuality)
	: Scene{ hwnd, SCENE_STAT::IN_GAME, (XMFLOAT4)Colors::White, netPtr }
{
	OutputDebugStringW(L"In Game Scene Entered.\n");

	mMsaa4xQualityLevels = msaaQuality;
	mMsaa4xEnable = msaaEnable;
	mKeyMap[VK_LEFT] = false;
	mKeyMap[VK_RIGHT] = false;
	mKeyMap[VK_UP] = false;
	mKeyMap[VK_DOWN] = false;
	mKeyMap[VK_LSHIFT] = false;
	mKeyMap[VK_F10] = false;
	mKeyMap[VK_LCONTROL] = false;
	mKeyMap['Z'] = false;
	mKeyMap['X'] = false;
	mKeyMap['P'] = false;

#ifdef STANDALONE
	mGameStarted = true;
#else
	mGameStarted = false;
#endif
}

InGameScene::~InGameScene()
{
}

void InGameScene::OnResize(float aspect)
{
	if (mMainCamera)
		mMainCamera->SetLens(aspect);
	if (mDirectorCamera)
		mDirectorCamera->SetLens(aspect);

	CreateMsaaViews();
	CreateVelocityMapViews();

	ComputePipeline* p = mPostProcessingPipelines[Layer::Bloom].get();
	auto bloom = dynamic_cast<BloomPipeline*>(p);
	bloom->CreateTextures(mDevice);
	bloom->BuildSRVAndUAV(mDevice);

	p = mPostProcessingPipelines[Layer::MotionBlur].get();
	auto motionBlur = dynamic_cast<MotionBlurPipeline*>(p);
	motionBlur->CreateTextures(mDevice);
	motionBlur->BuildSRVAndUAV(mDevice);

	p = mPostProcessingPipelines[Layer::VolumetricScattering].get();
	auto volumetric = dynamic_cast<VolumetricScatteringPipeline*>(p);
	volumetric->CreateTextures(mDevice);
	volumetric->BuildSRVAndUAV(mDevice);
}

void InGameScene::BuildObjects(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer,
	float Width, float Height, float aspect,
	const shared_ptr<BulletWrapper>& physics)
{
	mDevice = device;

	mpUI = std::make_unique<InGameUI>(nFrame, mDevice, cmdQueue);
	mpUI.get()->BuildObjects(backBuffer, static_cast<UINT>(Width), static_cast<UINT>(Height));

	mMainCamera = make_unique<Camera>();
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 1500.0f);
	mMainCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	mMainCamera->SetPosition(0.0f, 0.0f, 0.0f);
	mMainCamera->Move(mMainCamera->GetLook(), -mCameraRadius);

	mDirectorCamera = make_unique<Camera>();
	mDirectorCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 20000.0f);
	mDirectorCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	mDirectorCamera->SetPosition(500.0f, 50.0f, 500.0f);

	mCurrentCamera = mDirectorCamera.get();

	mMainLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	XMFLOAT3 direction = XMFLOAT3(-1.0f, 0.75f, -1.0f);
#ifdef STANDALONE

	mDirectionalLight.light.SetInfo(
		XMFLOAT3(0.2f, 0.2f, 0.2f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		direction,
		0.0f, 0.0f, 0.0f,
		3000.0f, DIRECTIONAL_LIGHT);

	VolumetricInfo v;

	v.Direction = direction;
	v.Position = { 0.0f, 0.0f, 0.0f };
	v.Range = 0;
	v.VolumetricStrength = 0.1f;
	v.outerCosine = 0;
	v.innerCosine = 0;
	v.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	v.Type = DIRECTIONAL_LIGHT;

	mDirectionalLight.volumetric = v;

#else
	if (mNetPtr->GetMapIndex() == 0)
	{
		mDirectionalLight.light.SetInfo(
			XMFLOAT3(0.7f, 0.7f, 0.7f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			direction,
			0.0f, 0.0f, 0.0f,
			3000.0f, DIRECTIONAL_LIGHT);

		VolumetricInfo v;

		v.Direction = direction;
		v.Position = { 0.0f, 0.0f, 0.0f };
		v.Range = 0;
		v.VolumetricStrength = 0.5f;
		v.outerCosine = 0;
		v.innerCosine = 0;
		v.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		v.Type = DIRECTIONAL_LIGHT;

		mDirectionalLight.volumetric = v;
	}
	else if (mNetPtr->GetMapIndex() == 1)
	{
		mDirectionalLight.light.SetInfo(
			XMFLOAT3(0.2f, 0.2f, 0.2f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			direction,
			0.0f, 0.0f, 0.0f,
			3000.0f, DIRECTIONAL_LIGHT);

		VolumetricInfo v;

		v.Direction = direction;
		v.Position = { 0.0f, 0.0f, 0.0f };
		v.Range = 0;
		v.VolumetricStrength = 0.1f;
		v.outerCosine = 0;
		v.innerCosine = 0;
		v.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		v.Type = DIRECTIONAL_LIGHT;

		mDirectionalLight.volumetric = v;
	}
#endif

	BuildRootSignature();
	BuildComputeRootSignature();
	BuildShadersAndPSOs(cmdList);
	BuildGameObjects(cmdList, physics);
	BuildConstantBuffers();
	BuildDescriptorHeap();

	BuildParticleObject(cmdList);

	//Sound
	SetSound();
	//Listener
	BuildListener(mCurrentCamera->GetPosition(), mCurrentCamera->GetLook(), mCurrentCamera->GetUp());

	// Let server know that loading sequence is done.
#ifndef STANDALONE
	mNetPtr->Client()->SendLoadSequenceDone(mNetPtr->GetRoomID());
	/*mNetPtr->StarttHolePunching();*/
#endif
}

void InGameScene::BuildRootSignature()
{
	D3D12_DESCRIPTOR_RANGE descRanges[4];
	descRanges[0] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4);
	descRanges[1] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
	descRanges[2] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1);
	descRanges[3] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);

	D3D12_ROOT_PARAMETER parameters[10];
	parameters[0] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 0, D3D12_SHADER_VISIBILITY_ALL);    // CameraCB
	parameters[1] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);    // LightCB
	parameters[2] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 2, D3D12_SHADER_VISIBILITY_ALL);    // GameInfoCB
	parameters[3] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 3, D3D12_SHADER_VISIBILITY_ALL);	 // MaterialCB
	parameters[4] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);			     // Object,  CBV
	parameters[5] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_ALL);				 // Texture, SRV
	parameters[6] = Extension::DescriptorTable(1, &descRanges[2], D3D12_SHADER_VISIBILITY_ALL);				 // ShadowMap
	parameters[7] = Extension::DescriptorTable(1, &descRanges[3], D3D12_SHADER_VISIBILITY_ALL);				 // CubeMap
	parameters[8] = Extension::Constants(7, 5, D3D12_SHADER_VISIBILITY_ALL);                                 // E.T.C - 기타 바로바로 올려야 할 필요가 있는 쉐이더 상수들
	parameters[9] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_ALL, 3); // Instancing Structured Buffer

	D3D12_STATIC_SAMPLER_DESC samplerDesc[5];
	samplerDesc[0] = Extension::SamplerDesc(0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	samplerDesc[1] = Extension::SamplerDesc(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	samplerDesc[2] = Extension::SamplerDesc(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	samplerDesc[3] = Extension::SamplerDesc(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	samplerDesc[4] = Extension::SamplerDesc(4,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = Extension::RootSignatureDesc(_countof(parameters), parameters,
		_countof(samplerDesc), samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT);

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	ThrowIfFailed(D3D12SerializeRootSignature(
		&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		rootSigBlob.GetAddressOf(), errorBlob.GetAddressOf()));

	ThrowIfFailed(mDevice->CreateRootSignature(
		0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void InGameScene::BuildComputeRootSignature()
{
	D3D12_DESCRIPTOR_RANGE descRanges[3];
	descRanges[0] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
	descRanges[1] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	descRanges[2] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 2);

	D3D12_ROOT_PARAMETER parameters[5];
	parameters[0] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);    // Inputs
	parameters[1] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_ALL);    // Output																   
	parameters[2] = Extension::Constants(6, 0, D3D12_SHADER_VISIBILITY_ALL);					   // 32bit Constant
	parameters[3] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);    // CameraCB
	parameters[4] = Extension::DescriptorTable(1, &descRanges[2], D3D12_SHADER_VISIBILITY_ALL); //Shadow Map

	D3D12_STATIC_SAMPLER_DESC samplerDesc[2];
	samplerDesc[0] = Extension::SamplerDesc(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, D3D12_SHADER_VISIBILITY_ALL);
	samplerDesc[1] = Extension::SamplerDesc(1, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = Extension::RootSignatureDesc(_countof(parameters), parameters,
		_countof(samplerDesc), samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	ThrowIfFailed(D3D12SerializeRootSignature(
		&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		rootSigBlob.GetAddressOf(), errorBlob.GetAddressOf()));

	ThrowIfFailed(mDevice->CreateRootSignature(
		0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&mComputeRootSignature)));
}

void InGameScene::BuildShadersAndPSOs(ID3D12GraphicsCommandList* cmdList)
{
	//auto defaultShader = make_unique<DefaultShader>(L"Shaders\\default.hlsl");
	//auto instancingShader = make_unique<DefaultShader>(L"Shaders\\Instancing.hlsl");
	//auto colorShader = make_unique<DefaultShader>(L"Shaders\\color.hlsl");
	////auto terrainShader = make_unique<TerrainShader>(L"Shaders\\terrain.hlsl");
	//auto motionBlurShader = make_unique<ComputeShader>(L"Shaders\\motionBlur.hlsl");
	//auto simpleShader = make_unique<DefaultShader>(L"Shaders\\simple.hlsl");
	//auto particleShader = make_unique<BillboardShader>(L"Shaders\\billboard.hlsl", true);
	//auto downSampleShader = make_unique<ComputeShader>(L"Shaders\\thresholdDownSample.hlsl");
	//auto blurShader = make_unique<ComputeShader>(L"Shaders\\blur.hlsl");
	//auto bloomMergeShader = make_unique<ComputeShader>(L"Shaders\\bloomMerge.hlsl");
	//auto volumetricScatteringShader = make_unique<ComputeShader>(L"Shaders\\volumetricScattering.hlsl");

	auto defaultShader = make_unique<DefaultShader>(L"Shaders\\default_VS.cso", L"Shaders\\default_PS.cso");
	auto instancingShader = make_unique<DefaultShader>(L"Shaders\\Instancing_VS.cso", L"Shaders\\Instancing_PS.cso");
	auto colorShader = make_unique<DefaultShader>(L"Shaders\\color_VS.cso", L"Shaders\\color_PS.cso");
	auto simpleShader = make_unique<DefaultShader>(L"Shaders\\simple_VS.cso", L"Shaders\\simple_PS.cso");
	//auto terrainShader = make_unique<TerrainShader>(L"Shaders\\terrain.hlsl");
	//auto particleShader = make_unique<BillboardShader>(L"Shaders\\billboard_VS.cso", L"Shaders\\billboard_GS.cso", L"Shaders\\billboard_PS.cso", true);
	auto motionBlurShader = make_unique<ComputeShader>(L"Shaders\\motionBlur.cso", true);
	auto downSampleShader = make_unique<ComputeShader>(L"Shaders\\thresholdDownSample.cso", true);
	auto blurShader = make_unique<ComputeShader>(L"Shaders\\blur.cso", true);
	auto bloomMergeShader = make_unique<ComputeShader>(L"Shaders\\bloomMerge.cso", true);
	auto volumetricScatteringShader = make_unique<ComputeShader>(L"Shaders\\volumetricScattering.cso", true);

	mPipelines[Layer::Default] = make_unique<Pipeline>();
	//mPipelines[Layer::Terrain] = make_unique<Pipeline>();

#ifdef STANDALONE
	mPipelines[Layer::SkyBox] = make_unique<SkyboxPipeline>(mDevice.Get(), cmdList, 1);
#else
	mPipelines[Layer::SkyBox] = make_unique<SkyboxPipeline>(mDevice, cmdList, mNetPtr->GetMapIndex());
#endif

	mPipelines[Layer::Instancing] = make_unique<InstancingPipeline>();
	mPipelines[Layer::Color] = make_unique<Pipeline>();
	mPipelines[Layer::Transparent] = make_unique<InstancingPipeline>();
	mPipelines[Layer::CheckPoint] = make_unique<Pipeline>();
	mPipelines[Layer::DriftParticle] = make_unique<StreamOutputPipeline>(2);
	mPipelines[Layer::MissileParticle] = make_unique<StreamOutputPipeline>(10);

	mShadowMapRenderer = make_unique<ShadowMapRenderer>(mDevice, 5000, 5000, 3, mCurrentCamera, mDirectionalLight.light.Direction);

	if (mMsaa4xEnable)
	{
		for (const auto& [_, pso] : mPipelines)
		{
			if (pso)
				pso->SetMsaa(mMsaa4xEnable, mMsaa4xQualityLevels);
		}
	}

	mPipelines[Layer::Default]->BuildPipeline(mDevice, mRootSignature.Get(), defaultShader.get());

	mPipelines[Layer::DriftParticle]->BuildPipeline(mDevice, mRootSignature.Get(), nullptr);
	mPipelines[Layer::MissileParticle]->BuildPipeline(mDevice, mRootSignature.Get(), nullptr);

	//mPipelines[Layer::Terrain]->SetWiredFrame(true);
	//mPipelines[Layer::Terrain]->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
	//mPipelines[Layer::Terrain]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), terrainShader.get());

	mPipelines[Layer::Color]->SetAlphaBlending();
	mPipelines[Layer::Color]->BuildPipeline(mDevice, mRootSignature.Get(), colorShader.get());

	mPipelines[Layer::Instancing]->BuildPipeline(mDevice, mRootSignature.Get(), instancingShader.get());

	mPipelines[Layer::SkyBox]->BuildPipeline(mDevice, mRootSignature.Get());

	mPipelines[Layer::Transparent]->SetAlphaBlending();
	mPipelines[Layer::Transparent]->BuildPipeline(mDevice, mRootSignature.Get(), instancingShader.get());

	mPipelines[Layer::CheckPoint]->SetWiredFrame(true);
	mPipelines[Layer::CheckPoint]->BuildPipeline(mDevice, mRootSignature.Get(), simpleShader.get());

	mPostProcessingPipelines[Layer::MotionBlur] = make_unique<MotionBlurPipeline>();
	mPostProcessingPipelines[Layer::MotionBlur]->BuildPipeline(mDevice, mComputeRootSignature.Get(), motionBlurShader.get(), true);

	mPostProcessingPipelines[Layer::Bloom] = make_unique<BloomPipeline>();
	mPostProcessingPipelines[Layer::Bloom]->BuildPipeline(mDevice, mComputeRootSignature.Get(), downSampleShader.get(), true);
	mPostProcessingPipelines[Layer::Bloom]->BuildPipeline(mDevice, mComputeRootSignature.Get(), blurShader.get());
	mPostProcessingPipelines[Layer::Bloom]->BuildPipeline(mDevice, mComputeRootSignature.Get(), bloomMergeShader.get());

	mPostProcessingPipelines[Layer::VolumetricScattering] = make_unique<VolumetricScatteringPipeline>();
	mPostProcessingPipelines[Layer::VolumetricScattering]->BuildPipeline(mDevice, mComputeRootSignature.Get(), volumetricScatteringShader.get(), true);

	mShadowMapRenderer->AppendTargetPipeline(Layer::Default, mPipelines[Layer::Default].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Color, mPipelines[Layer::Color].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Instancing, mPipelines[Layer::Instancing].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Transparent, mPipelines[Layer::Transparent].get());
	mShadowMapRenderer->BuildPipeline(mDevice, mRootSignature.Get());
}

void InGameScene::BuildConstantBuffers()
{
	mLightCB = std::make_unique<ConstantBuffer<LightConstants>>(mDevice, 2);
	mCameraCB = std::make_unique<ConstantBuffer<CameraConstants>>(mDevice, 10); // 메인 카메라 1개, 그림자 매핑 카메라 3개, 다이나믹 큐브매핑 카메라 6개
	mGameInfoCB = std::make_unique<ConstantBuffer<GameInfoConstants>>(mDevice, 1);
	mVolumetricCB = std::make_unique<ConstantBuffer<VolumetricConstants>>(mDevice, 1);

	for (const auto& [_, pso] : mPipelines)
	{
		if (pso)
			pso->BuildConstantBuffer(mDevice);
	}
}

void InGameScene::BuildDescriptorHeap()
{
	CreateVelocityMapDescriptorHeaps();
	CreateVelocityMapViews();

	CreateMsaaDescriptorHeaps();
	CreateMsaaViews();

	mShadowMapRenderer->BuildDescriptorHeap(mDevice, 3, 4, 5);
	for (const auto& [_, pso] : mPipelines)
		pso->BuildDescriptorHeap(mDevice, 3, 4, 5);
}

void InGameScene::CreateVelocityMapViews()
{
	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R32G32B32A32_FLOAT, {0.0f,0.0f,0.0f,0.0f} };

	mMsaaVelocityMap = CreateTexture2DResource(
		mDevice, gFrameWidth, gFrameHeight, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, 4, mMsaa4xQualityLevels - 1);

	mMsaaVelocityMap->SetName(L"MSAA Velocity Map");

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mMsaaVelocityMapRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	mDevice->CreateRenderTargetView(mMsaaVelocityMap.Get(), &rtvDesc, rtvHandle);
	mMsaaVelocityMapRtvHandle = rtvHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = mMsaaVelocityMapSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mDevice->CreateShaderResourceView(mMsaaVelocityMap.Get(), &srvDesc, srvHandle);
	mMsaaVelocityMapSrvHandle = srvHandle;
}

void InGameScene::CreateVelocityMapDescriptorHeaps()
{
	ThrowIfFailed(mDevice->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			1,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
		IID_PPV_ARGS(&mMsaaVelocityMapRtvDescriptorHeap)));

	ThrowIfFailed(mDevice->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			1,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mMsaaVelocityMapSrvDescriptorHeap)));
}

void InGameScene::CreateMsaaDescriptorHeaps()
{
	ThrowIfFailed(mDevice->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			1,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
		IID_PPV_ARGS(&mMsaaRtvDescriptorHeap)));
}

void InGameScene::CreateMsaaViews()
{
	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f,0.0f,0.0f,0.0f} };

	mMsaaTarget = CreateTexture2DResource(
		mDevice, gFrameWidth, gFrameHeight, 1, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, 4, mMsaa4xQualityLevels - 1);

	mMsaaTarget->SetName(L"MSAA Target");

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mMsaaRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	mDevice->CreateRenderTargetView(mMsaaTarget.Get(), &rtvDesc, rtvHandle);
	mMsaaRtvHandle = rtvHandle;
}

void InGameScene::BuildGameObjects(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics)
{
	//mDynamicsWorld = physics->GetDynamicsWorld();

	//mMeshList["Missile"].push_back(std::make_shared<BoxMesh>(mDevice.Get(), cmdList, 2.0f, 2.0f, 2.0f));

#ifdef STANDALONE
	LoadWorldMap(cmdList, physics, "Map\\MapData_night.tmap");
	LoadCheckPoint(cmdList, L"Map\\CheckPoint_night.tmap");
	LoadLights(cmdList, L"Map\\Lights_night.tmap");
#else
	if (mNetPtr->GetMapIndex() == 0)
	{
		LoadWorldMap(cmdList, physics, "Map\\MapData_day.tmap");
		LoadCheckPoint(cmdList, L"Map\\CheckPoint_day.tmap");
		//LoadLights(cmdList, L"Map\\Lights_day.tmap");
	}
	else if (mNetPtr->GetMapIndex() == 1)
	{
		LoadWorldMap(cmdList, physics, "Map\\MapData_night.tmap");
		LoadCheckPoint(cmdList, L"Map\\CheckPoint_night.tmap");
		LoadLights(cmdList, L"Map\\Lights_night.tmap");
	}
#endif

#ifdef STANDALONE
	BuildCarObject({ -306.5f, 1.0f, 253.7f }, { 0.0f, 0.707107f, 0.0f, -0.707107f }, 0, true, cmdList, physics, 0);
#else
	int playerCount = 0;
	const auto& players = mNetPtr->GetPlayersInfo();
	for (int i = 0; const PlayerInfo & info : players)
	{
		if (info.Empty == false)
		{
			bool isPlayer = (i == mNetPtr->GetPlayerIndex()) ? true : false;
			BuildCarObject(info.StartPosition, info.StartRotation, info.Color, isPlayer, cmdList, physics, i);
			BuildMissileObject(cmdList, info.StartPosition, i);
			playerCount += 1;
		}
		i++;
	}
	mpUI->SetPlayerCount(playerCount);

#endif
	float aspect = mMainCamera->GetAspect();
	mMainCamera.reset(mPlayer->ChangeCameraMode((int)CameraMode::THIRD_PERSON_CAMERA));
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 1500.0f);
	mMainCamera->SetPosition(mPlayer->GetPosition());
	mMainCamera->SetRotation(mPlayer->GetQuaternion());
	mCurrentCamera = mMainCamera.get();
	//mCurrentCamera->Update(100.0f);
	// TEST
	//mDirectorCamera->SetPosition(mMainCamera->GetPosition());
	//mCurrentCamera = mDirectorCamera.get();
	//

	for (const auto& [_, pso] : mPipelines)
	{
		if (pso)
			pso->SortMeshes();
	}
}

void InGameScene::BuildCarObject(
	const XMFLOAT3& position,
	const XMFLOAT4& rotation,
	char color,
	bool isPlayer,
	ID3D12GraphicsCommandList* cmdList,
	const std::shared_ptr<BulletWrapper>& physics,
	UINT netID)
{
	auto carObj = make_shared<PhysicsPlayer>(netID);
	carObj->SetPosition(position);
	carObj->SetQuaternion(rotation);

	if (mMeshList["Car_Body.obj"].empty())
	{
		carObj->LoadModel(mDevice, cmdList, L"Models\\Car_Body.obj");
		mMeshList["Car_Body.obj"] = carObj->GetMeshes();
	}
	else
	{
		carObj->CopyMeshes(mMeshList["Car_Body.obj"]);
	}

	carObj->SetDiffuse("Car_Texture", mColorMap[(int)color]);
	for (int i = 0; i < 4; ++i)
	{
		auto wheelObj = make_shared<WheelObject>(*carObj.get());

		XMFLOAT3 wheelOffset = mWheelOffset;
		if (i >= 2)
		{
			wheelOffset.z *= -1.0f;
		}
		if (i % 2 == 0)
		{
			wheelOffset.x *= -1.0f;
		}
		wheelObj->SetLocalOffset(wheelOffset);

		if (i % 2 == 0)
		{
			if (mMeshList["Car_Wheel_L.obj"].empty())
			{
				wheelObj->LoadModel(mDevice, cmdList, L"Models\\Car_Wheel_L.obj");
				mMeshList["Car_Wheel_L.obj"] = wheelObj->GetMeshes();
			}
			else
			{
				wheelObj->CopyMeshes(mMeshList["Car_Wheel_L.obj"]);
			}
		}
		else
		{
			if (mMeshList["Car_Wheel_R.obj"].empty())
			{
				wheelObj->LoadModel(mDevice, cmdList, L"Models\\Car_Wheel_R.obj");
				mMeshList["Car_Wheel_R.obj"] = wheelObj->GetMeshes();
			}
			else
			{
				wheelObj->CopyMeshes(mMeshList["Car_Wheel_R.obj"]);
			}
		}
		carObj->SetWheel(wheelObj, i);
		mPipelines[Layer::Color]->AppendObject(wheelObj);
	}

#ifdef STANDALONE
	carObj->BuildRigidBody(physics);
#endif
	carObj->BuildDsvRtvView(mDevice);

	if (isPlayer) mPlayer = std::static_pointer_cast<Player>(carObj);
	mPipelines[Layer::Color]->AppendObject(carObj);
	mPlayerObjects[netID] = std::move(carObj);
}

void InGameScene::BuildMissileObject(
	ID3D12GraphicsCommandList* cmdList,
	const XMFLOAT3& position, int idx)
{
	mMissileObjects[idx] = std::make_shared<MissileObject>(position);

	if (mMeshList.find("Missile") == mMeshList.end())
	{
		mMissileObjects[idx]->LoadModel(mDevice, cmdList, L"Models\\Missile.obj");
		mMeshList["Missile"] = mMissileObjects[idx]->GetMeshes();
		mTextureList["Missile"] = mMissileObjects[idx]->GetTextures();
	}
	else
	{
		mMissileObjects[idx]->SetMeshes(mMeshList["Missile"]);
		mMissileObjects[idx]->SetTextures(mTextureList["Missile"]);
	}
}

void InGameScene::PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed)
{
	if (mShadowMapRenderer)
		mShadowMapRenderer->PreRender(cmdList, this);

	mPlayer->PreDraw(cmdList, this, mCubemapDrawIndex);

	if (mCubemapDrawIndex < 5)
		mCubemapDrawIndex++;
	else
	{
		mPlayer->ChangeCurrentRenderTarget();
		mCubemapDrawIndex = 0;
	}

	mPlayer->SetCubemapSrv(cmdList, 7);
}

bool InGameScene::ProcessPacket(std::byte* packet, const SC::PCK_TYPE& type, int bytes)
{
	switch (type)
	{
	case SC::PCK_TYPE::READY_SIGNAL:
	{
		SC::packet_ready_signal* pck = reinterpret_cast<SC::packet_ready_signal*>(packet);
		mpUI->ShowStartAnim();
		auto mapIndex = mNetPtr->GetMapIndex();
		switch (int(mapIndex))
		{
		case 0:
			GetSound().Play(0.2f, static_cast<int>(IngameUI_SOUND_TRACK::BGM1));
			break;
		case 1:
			GetSound().Play(0.2f, static_cast<int>(IngameUI_SOUND_TRACK::BGM2));
			break;
		default:
			GetSound().Play(0.2f, static_cast<int>(IngameUI_SOUND_TRACK::BGM1));
			break;
		}
		//GetSound().Play(0.2f, static_cast<int>(IngameUI_SOUND_TRACK::BGM1));
		//GetSound().Play(0.2f, static_cast<int>(IngameUI_SOUND_TRACK::BGM2));

		break;
	}
	case SC::PCK_TYPE::START_SIGNAL:
	{
		SC::packet_start_signal* pck = reinterpret_cast<SC::packet_start_signal*>(packet);
		mGameStarted = true;
		mpUI->ShowGoAnim();
		mpUI->SetRunningTime((float)pck->running_time_sec);
		
		break;
	}
	case SC::PCK_TYPE::REMOVE_PLAYER:
	{
		SC::packet_remove_player* pck = reinterpret_cast<SC::packet_remove_player*>(packet);
		mNetPtr->RemovePlayer(pck);

		const auto& player = mPlayerObjects[pck->player_idx];
		if (player)	player->SetUpdateFlag(UPDATE_FLAG::REMOVE);
		break;
	}
	case SC::PCK_TYPE::REMOVE_MISSILE:
	{
		SC::packet_remove_missile* pck = reinterpret_cast<SC::packet_remove_missile*>(packet);
		const auto& missile = mMissileObjects[pck->missile_idx];
		if (missile)
		{
			FMOD_VECTOR SoundPos{}, SoundVel{};

			auto& missilePos = missile->GetPosition();
			auto& missileVel = missile->GetLinearVelocity();
			SoundPos.x = missilePos.x;
			SoundPos.y = missilePos.y;
			SoundPos.z = missilePos.z;
			SoundVel.x = missileVel.GetXMFloat3().x;
			SoundVel.y = missileVel.GetXMFloat3().y;
			SoundVel.z = missileVel.GetXMFloat3().z;

			missile->SetUpdateFlag(UPDATE_FLAG::REMOVE);
			auto& sound = GetSound();
			sound.Play3D(static_cast<int>(IngameUI_SOUND_TRACK::MISSILE_EXPLOSION), SoundPos, SoundVel);
			
			//sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::MISSILE_EXPLOSION));
		}
		break;
	}
	case SC::PCK_TYPE::MEASURE_RTT:
	{
		SC::packet_measure_rtt* pck = reinterpret_cast<SC::packet_measure_rtt*>(packet);
		mNetPtr->SetLatency(pck->latency_ms);
		mNetPtr->CalcClockDelta(pck->s_send_time);
		mNetPtr->Client()->SendMeasureRTTPacket(pck->s_send_time);

		mNetPtr->SetHolePunchingDone(true);

		/*auto now = Clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			now - mPrevTimepoint).count();
		mPrevTimepoint = now;

		Log::Print("Server tick: ", duration);
		Log::Print("Latency: ", mNetPtr->GetLatency() * 1000.0f);*/
		break;
	}
	case SC::PCK_TYPE::PLAYER_TRANSFORM:
	{
		SC::packet_player_transform* pck = reinterpret_cast<SC::packet_player_transform*>(packet);
		const auto& player = mPlayerObjects[pck->player_idx];

		if (player)
		{
			if (pck->speed > 0)
			{
				// Set UI information
				if (player.get() == mPlayer)
					mpUI->SetSpeed(pck->speed);
				// Set current speed of player
				player->SetCurrentSpeed(pck->speed);
			}
			player->SetCorrectionTransform(pck, mNetPtr->GetServerTimeStamp(), mNetPtr->GetLatency());
		}
		break;
	}
	case SC::PCK_TYPE::MISSILE_LAUNCHED:
	{
		SC::packet_missile_launched* pck = reinterpret_cast<SC::packet_missile_launched*>(packet);
		auto& missile = mMissileObjects[pck->missile_idx];
		if (missile)
		{
			auto pos = Compressor::DecodePos(pck->position);
			auto quat = Compressor::DecodeQuat(pck->quaternion);
			missile->SetPosition(XMFLOAT3{ pos[0], pos[1], pos[2] });
			missile->SetQuaternion(XMFLOAT4{ quat[0], quat[1], quat[2], quat[3] });
			missile->SetUpdateFlag(UPDATE_FLAG::CREATE);
		}
		break;
	}
	case SC::PCK_TYPE::MISSILE_TRANSFORM:
	{
		SC::packet_missile_transform* pck = reinterpret_cast<SC::packet_missile_transform*>(packet);
		const auto& missile = mMissileObjects[pck->missile_idx];

		if (missile && missile->IsActive())
		{
			missile->SetCorrectionTransform(pck, mNetPtr->GetServerTimeStamp(), mNetPtr->GetLatency());
		}
		break;
	}
	case SC::PCK_TYPE::UI_INFO:
	{
		SC::packet_ui_info* pck = reinterpret_cast<SC::packet_ui_info*>(packet);
		const auto& player = mPlayerObjects[pck->player_idx];
		if (player)
		{
			if (pck->gauge > 0) mpUI->SetDriftGauge(pck->gauge);
		}
		break;
	}
	case SC::PCK_TYPE::INVINCIBLE_ON:
	{
		SC::packet_invincible_on* pck = reinterpret_cast<SC::packet_invincible_on*>(packet);
		const auto& player = mPlayerObjects[pck->player_idx];
		if (player)
		{
			int duration = pck->duration;
			player->SetInvincibleOn(duration);
		}
		break;
	}
	case SC::PCK_TYPE::ITEM_COUNT:
	{
		SC::packet_item_count* pck = reinterpret_cast<SC::packet_item_count*>(packet);
		const auto& player = mPlayerObjects[pck->player_idx];
		if (player)
		{
			OutputDebugStringA("Item increased.\n");
			player->SetItemNum(pck->item_count);
		}
		break;
	}
	case SC::PCK_TYPE::SPAWN_TRANSFORM:
	{
		SC::packet_spawn_transform* pck = reinterpret_cast<SC::packet_spawn_transform*>(packet);
		const auto& player = mPlayerObjects[pck->player_idx];
		if (player) player->SetSpawnTransform(pck);
		break;
	}
	case SC::PCK_TYPE::WARNING_MESSAGE:
	{
		SC::packet_warning_message* pck = reinterpret_cast<SC::packet_warning_message*>(packet);
		OutputDebugStringA("Reverse drive warning!\n");
		mpUI->ShowWarning();
		break;
	}
	case SC::PCK_TYPE::INGAME_INFO:
	{
		SC::packet_ingame_info* pck = reinterpret_cast<SC::packet_ingame_info*>(packet);
		const auto& player = mPlayerObjects[pck->player_idx];
		if (player.get() == mPlayer)
		{
			mpUI->SetLap(pck->lap_count);
			mpUI->SetMyRank(pck->rank);
			mpUI->SetMyScore(pck->point);
		}
		break;
	}
	case SC::PCK_TYPE::GAME_END:
	{
		SC::packet_game_end* pck = reinterpret_cast<SC::packet_game_end*>(packet);

		const auto& playerInfo = mNetPtr->GetPlayersInfo();
		mpUI->GetMutex().lock();
		for (int i = 0, idx = 0; i < mPlayerObjects.size(); i++)
		{
			if (mPlayerObjects[i])
			{
				mpUI->SetScoreboardInfo(
					idx, (int)pck->rank[i], pck->point[i],
					(int)pck->lap_count[i], (int)pck->hit_count[i],
					playerInfo[i].Name);
				idx += 1;
			}
		}
		mpUI->SortScoreboard();
		mpUI->GetMutex().unlock();
		for(int i=0;i<INGAME_SOUND_NUM;++i)
			GetSound().Stop(i);
		mpUI->ShowScoreBoard();
		mGameEnded = true;
		mRevertTime = Clock::now() + std::chrono::seconds(WAIT_TO_REVERT); // waits for 5 seconds before revert.
		break;
	}
	case SC::PCK_TYPE::ROOM_INSIDE_INFO:
	{
		OutputDebugString(L"Received room inside info packet.\n");
		SC::packet_room_inside_info* pck = reinterpret_cast<SC::packet_room_inside_info*>(packet);
		mNetPtr->InitRoomInfo(pck);
		break;
	}
	default:
		OutputDebugStringA("Invalid packet.\n");
		return false;
	}
	return true;
}

void InGameScene::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
	if ((buttonState) && !GetCapture())
	{
		SetCapture(mHwnd);
		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}
}

void InGameScene::OnProcessMouseUp(WPARAM buttonState, int x, int y)
{
	ReleaseCapture();
}

void InGameScene::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
	// 마우스로 화면 돌리는 기능
	if ((buttonState & MK_LBUTTON) && GetCapture())
	{
		float dx = static_cast<float>(x - mLastMousePos.x);
		float dy = static_cast<float>(y - mLastMousePos.y);

		mLastMousePos.x = x;
		mLastMousePos.y = y;

		mDirectorCamera->Pitch(0.25f * dy);
		mDirectorCamera->RotateY(0.25f * dx);
	}
	mpUI.get()->OnProcessMouseMove(buttonState, x, y);
}

void InGameScene::OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto sound = GetSound();
	switch (uMsg)
	{
	case WM_KEYUP:
		if (wParam == 'G')
		{
			if (mCurrentCamera == mDirectorCamera.get())
				mCurrentCamera = mMainCamera.get();
			else
			{
				mDirectorCamera->SetPosition(mMainCamera->GetPosition());
				mCurrentCamera = mDirectorCamera.get();
			}
		}
		if (wParam == 'M')
		{
			mMotionBlurEnable = !mMotionBlurEnable;
		}
		if (wParam == 'N')
		{
			mCheckPointEnable = !mCheckPointEnable;
		}
		if (wParam == 'B')
		{
			mBloomEnable = !mBloomEnable;
		}
		if (wParam == 'V')
		{
			mVolumetricEnable = !mVolumetricEnable;
		}
		if (wParam == VK_SHIFT)
		{
			for (int i = 0; i < mPipelines[Layer::DriftParticle]->GetRenderObjects().size(); ++i)
			{
				std::shared_ptr<SOParticleObject>& obj = std::static_pointer_cast<SOParticleObject>(mPipelines[Layer::DriftParticle]->GetRenderObjects()[i]);
				obj->SetParticleEnable(false);
			}
			auto velocity = mpUI.get()->GetSpeed();
			auto& sound = GetSound();
			const auto& channel = sound.GetChannel();
			FMOD_RESULT res{};
			if (sound.GetIsDrift())
			{
				sound.SetIsDrift();
				FMOD_Channel_SetPaused(sound.GetChannel()[static_cast<int>(IngameUI_SOUND_TRACK::DRIFT_ORIGIN)], true);
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_SHIFT)
		{
			for (int i = 0; i < mPipelines[Layer::DriftParticle]->GetRenderObjects().size(); ++i)
			{
				std::shared_ptr<SOParticleObject>& obj = std::static_pointer_cast<SOParticleObject>(mPipelines[Layer::DriftParticle]->GetRenderObjects()[i]);
				obj->SetParticleEnable(true);
			}
			auto velocity = mpUI.get()->GetSpeed();
			auto& sound = GetSound();
			const auto& channel = sound.GetChannel();
			FMOD_RESULT res{};
			if (!sound.GetIsDrift() && velocity >= MIN_DRIFT_SOUND_SPEED)
			{
				sound.SetIsDrift();
				sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::DRIFT_ORIGIN));
				FMOD_Channel_SetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIFT_ORIGIN)], 1.0f);
				//FMOD_Channel_SetPosition(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIFT_ORIGIN)], static_cast<unsigned int>(DRIFT_SOUND_FRAME * 0.355f), FMOD_TIMEUNIT_PCM);
			}
		}
		if ((wParam == 'Z' || wParam == 'X'))
		{
			auto item = mPlayer->GetItemNum();
			if (item > 0)
			{
				mPlayer->SetItemNum(item - 1);

				if (wParam == 'Z')
				{
					mPlayer->SetBooster();
					mPlayer->SetRimLight(true);
					auto& sound = GetSound();
					const auto& channel = sound.GetChannel();
					FMOD_RESULT res{};
					sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::BOOSTER));
					sound.Play(MAX_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::BOOSTERBACK));
					//FMOD_Channel_SetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::BOOSTER)], 1.2f);
					//FMOD_Channel_SetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::BOOSTERBACK)], 1.2f);
				}
				else if (wParam == 'X')
				{
					auto& sound = GetSound();
					//Missile Pos, Vel Set to Sound
					FMOD_VECTOR SoundPos{}, SoundVel{};
					for (int i = 0; i < mMissileObjects.size(); ++i)
					{
						auto Missile = mMissileObjects[i].get();
						if (Missile == nullptr)
							continue;
						auto& missilePos = Missile->GetPosition();
						auto& missileVel = Missile->GetLinearVelocity();
						SoundPos.x = missilePos.x;
						SoundPos.y = missilePos.y;
						SoundPos.z = missilePos.z;
						SoundVel.x = missileVel.GetXMFloat3().x;
						SoundVel.y = missileVel.GetXMFloat3().y;
						SoundVel.z = missileVel.GetXMFloat3().z;
					}
					//sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::MISSILE));
					sound.Play3D(static_cast<int>(IngameUI_SOUND_TRACK::MISSILE), SoundPos, SoundVel);
				}
			}
		}
		break;
	}
	mpUI->OnProcessKeyInput(uMsg, wParam, lParam);
}

void InGameScene::OnPreciseKeyInput(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, float elapsed)
{
	if (mHwnd != GetFocus()) return;

	if (mCurrentCamera == mDirectorCamera.get())
	{
		const float dist = 50.0f;
		if (GetAsyncKeyState('A') & 0x8000)
			mDirectorCamera->Strafe(-dist * elapsed);
		if (GetAsyncKeyState('D') & 0x8000)
			mDirectorCamera->Strafe(dist * elapsed);
		if (GetAsyncKeyState('W') & 0x8000)
			mDirectorCamera->Walk(dist * elapsed);
		if (GetAsyncKeyState('S') & 0x8000)
			mDirectorCamera->Walk(-dist * elapsed);
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
			mDirectorCamera->Upward(dist * elapsed);
		if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
			mDirectorCamera->Upward(-dist * elapsed);
	}
	if (mPlayer) mPlayer->OnPreciseKeyInput(elapsed);

#ifndef STANDALONE
	for (auto& [key, val] : mKeyMap)
	{
		auto input = GetAsyncKeyState(key);

		if (input && val == false)
		{
			// Key pressed
			val = true;
			mNetPtr->Client()->SendKeyInput(mNetPtr->GetRoomID(), key, val);

		}
		else if (input == 0 && val == true)
		{
			// Key released
			val = false;
			mNetPtr->Client()->SendKeyInput(mNetPtr->GetRoomID(), key, val);

		}
	}

	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{

		auto velocity = mpUI.get()->GetSpeed();
		std::wstring text{ std::to_wstring(velocity) };
		//OutputDebugStringW(text.c_str());
		auto& sound = GetSound();
		if (sound.GetIsDecelerating())
			sound.SetIsDecelerating();

		const auto& channel = sound.GetChannel();
		FMOD_Channel_SetPaused(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_REVERSE)], true);

		FMOD_RESULT res{};
		FMOD_BOOL isPlaying = false;
		FMOD_Channel_IsPlaying(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], &isPlaying);

		if (!sound.GetIsDriving() && velocity > MIN_DRIVING_SOUND_SPEED)
		{
			sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN));
			//auto pos = static_cast<unsigned int>((DRIVING_SOUND_FRAME * DRIVING_SOUND_RUNNING_TIME) * (velocity / static_cast<float>(MAX_SPEED)));
			//if (pos > 78112) pos = 78112;



			//FMOD_Channel_SetPosition(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], pos, FMOD_TIMEUNIT_PCM);

			//FMOD_Channel_SetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], 0.8f + (1.0f * (velocity * (0.3f / MAX_SPEED))));
			sound.SetIsDrivingTrue();
		}
		else if (!isPlaying) // (driving == true || velocity<3) && isplaying == false
		{
			sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN));
			//auto pos = static_cast<unsigned int>((DRIVING_SOUND_FRAME * DRIVING_SOUND_RUNNING_TIME) * (velocity / static_cast<float>(MAX_SPEED)));
			//if (pos > 78112) pos = 78112;
			//res = FMOD_Channel_SetPosition(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], pos, FMOD_TIMEUNIT_PCM);
			//FMOD_Channel_SetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], 0.5f + (1.0f * (velocity * (0.7f / MAX_SPEED))));
		}
	}


	if (GetAsyncKeyState(VK_DOWN) & 0x8001 || GetAsyncKeyState(VK_DOWN) & 8000)
	{
		auto velocity = mpUI.get()->GetSpeed();
		auto& sound = GetSound();
		const auto& channel = sound.GetChannel();
		FMOD_RESULT res{};
		if (!sound.GetIsDecelerating())
		{
			sound.SetIsDecelerating();
			//sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::BIKE_BRAKE));

			auto pos = static_cast<unsigned int>((BIKE_BRAKE_SOUND_FRAME * BIKE_BRAKE_SOUND_RUNNING_TIME) * (MAX_SPEED - velocity) / static_cast<float>(MAX_SPEED));
			res = FMOD_Channel_SetPosition(channel[static_cast<int>(IngameUI_SOUND_TRACK::BIKE_BRAKE)], pos, FMOD_TIMEUNIT_PCM);

			if (velocity > 50)
			{
				//sound.Play(NORMAL_VOLUME, static_cast<int>(IngameUI_SOUND_TRACK::BRAKE_SKID));

				auto pos = static_cast<unsigned int>((BRAKE_SKID_SOUND_FRAME * BRAKE_SKID_SOUND_RUNNING_TIME) * (MAX_SPEED - velocity) / static_cast<float>(MAX_SPEED));
				res = FMOD_Channel_SetPosition(channel[static_cast<int>(IngameUI_SOUND_TRACK::BRAKE_SKID)], pos, FMOD_TIMEUNIT_PCM);
			}
		}
	}
	else
	{
		auto velocity = mpUI.get()->GetSpeed();
		auto& sound = GetSound();
		const auto& channel = sound.GetChannel();
		FMOD_RESULT res{};
		if (sound.GetIsDecelerating())
		{
			//FMOD_Channel_SetPaused(sound.GetChannel()[static_cast<int>(IngameUI_SOUND_TRACK::BIKE_BRAKE)], true);
			//FMOD_Channel_SetPaused(sound.GetChannel()[static_cast<int>(IngameUI_SOUND_TRACK::BRAKE_SKID)], true);
		}
	}

	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{

	}
	else
	{

	}

#endif
}

void InGameScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, const std::shared_ptr<BulletWrapper>& physics)
{
	if (mGameEnded)
	{
		auto now = Clock::now();
		if (now >= mRevertTime)
		{
			mNetPtr->Client()->RevertScene();
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::POP);
		}
	}

	float elapsed = timer.ElapsedTime();

#ifdef STANDALONE
	if (mGameStarted)
		physics->StepSimulation(elapsed);
#endif
	//physics->StepSimulation(elapsed);

	UpdatePlayerObjects();
	UpdateMissileObject();
	OnPreciseKeyInput(cmdList, physics, elapsed);

	UpdateLight(elapsed);

	mShadowMapRenderer->UpdateSplitFrustum(mCurrentCamera);
	mShadowMapRenderer->UpdateDepthCamera(cmdList);

	for (const auto& [_, pso] : mPipelines)
		pso->Update(elapsed, mNetPtr->GetClockDelta(), mCurrentCamera);
	mMainCamera->Update(elapsed);
	mDirectorCamera->Update(elapsed);

	UpdateConstants(timer);

	auto& sound = GetSound();
	// Set 3dSound Listener Attributes from Camera
	Update3DSound();

	sound.Update();

	auto velocity = mpUI.get()->GetSpeed();

	const auto& channel = sound.GetChannel();
	//FMOD_RESULT res;
	//FMOD_Channel_SetPosition(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], 48000 * 0.016f, FMOD_TIMEUNIT_PCM);
	if (velocity < MIN_DRIVING_SOUND_SPEED)
	{
		sound.SetIsDrivingFalse();
		FMOD_Channel_SetPaused(sound.GetChannel()[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], true);
	}
	FMOD_Channel_SetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], 0.3f + (1.0f * (velocity * (0.8f / MAX_SPEED))));

	float pitch{};
	FMOD_Channel_GetPitch(channel[static_cast<int>(IngameUI_SOUND_TRACK::DRIVING_ORIGIN)], &pitch);
	//OutputDebugStringW(std::to_wstring(pitch).c_str());

	mpUI.get()->Update(elapsed, mPlayer);
}

void InGameScene::UpdateLight(float elapsed)
{
}

void InGameScene::BuildParticleObject(ID3D12GraphicsCommandList* cmdList)
{
	if (mPipelines[Layer::DriftParticle]->GetRenderObjects().size() == 0)
	{
		XMFLOAT3 offset[2] = { XMFLOAT3(-1.3f, -0.8f, -1.8f), XMFLOAT3(1.3f, -0.8f, -1.8f) };

		for (int i = 0; i < 2; ++i)
		{
			auto obj = std::make_shared<SOParticleObject>(*mPlayer);

			auto particleEmittor = std::make_shared<ParticleMesh>(mDevice, cmdList, XMFLOAT3(0.6f, 0.3f, 0.0f), XMFLOAT4(0.6f, 0.3f, 0.0f, 1.0f), XMFLOAT2(0.1f, 0.1f), XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3(0.0f, -10.0f, -5.0f), 0.02f, 100);
			if (mTextureList["DriftParticle"].empty())
			{
				obj->LoadTexture(mDevice, cmdList, L"Resources\\Particle.dds", D3D12_SRV_DIMENSION_TEXTURE2D);
				mTextureList["DriftParticle"] = obj->GetTextures();
			}
			else
			{
				obj->SetTextures(mTextureList["DriftParticle"]);
			}

			obj->SetMesh(particleEmittor);
			obj->SetLocalOffset(offset[i]);

			Pipeline* p = mPipelines[Layer::DriftParticle].get();
			auto particlePipeline = dynamic_cast<StreamOutputPipeline*>(p);
			particlePipeline->AppendObject(mDevice, obj);
		}
	}


#ifndef STANDALONE

	const auto& players = mNetPtr->GetPlayersInfo();
	for (int i = 0; const PlayerInfo & info : players)
	{
		if (info.Empty == false)
		{
			XMFLOAT3 offset = XMFLOAT3(0.0f, 0.0f, -3.0f);

			auto particle = std::make_shared<SOParticleObject>(*mMissileObjects[i]);

			auto particleEmittor = std::make_shared<ParticleMesh>(mDevice, cmdList, XMFLOAT3(0, 0, 0), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 0.1f, 100);
			if (mTextureList["MissileParticle"].empty())
			{
				particle->LoadTexture(mDevice, cmdList, L"Resources\\MissileTrail.dds", D3D12_SRV_DIMENSION_TEXTURE2D);
				mTextureList["MissileParticle"] = particle->GetTextures();
			}
			else
			{
				particle->SetTextures(mTextureList["MissileParticle"]);
			}

			particle->SetMesh(particleEmittor);
			particle->SetLocalOffset(offset);

			mMissileObjects[i]->SetParticle(particle);

			Pipeline* p = mPipelines[Layer::MissileParticle].get();
			auto particlePipeline = dynamic_cast<StreamOutputPipeline*>(p);
			particlePipeline->AppendObject(mDevice, particle);

			i++;
		}
	}
#endif
}

void InGameScene::DestroyDriftParticleObject()
{
	mPipelines[Layer::DriftParticle]->GetRenderObjects().clear();
}

void InGameScene::UpdateLightConstants()
{
	for (int i = 0; i < mShadowMapRenderer->GetMapCount(); ++i)
		mMainLight.ShadowTransform[i] = Matrix4x4::Transpose(mShadowMapRenderer->GetShadowTransform(i));

	auto playerPos = mPlayer->GetPosition();


#ifndef STANDALONE
	for (auto i = mLights.begin(); i < mLights.end();)
	{
		if (i->light.pad0 == 1.0f)
			i = mLights.erase(i);
		else
			++i;
	}

	if (mNetPtr->GetMapIndex() == 1)
	{
		for (int i = 0; i < mPlayerObjects.size(); ++i)
		{
			if (mPlayerObjects[i])
			{
				LightBundle* frontLights;
				frontLights = mPlayerObjects[i]->GetLightBundle();

				mLights.push_back(frontLights[0]);
				mLights.push_back(frontLights[1]);
			}
		}
	}
#endif

	std::sort(mLights.begin(), mLights.end(),
		[playerPos](const LightBundle& l1, const LightBundle& l2)
		{
			return Vector3::Distance(l1.light.Position, playerPos) < Vector3::Distance(l2.light.Position, playerPos);
		}
	);

	for (int i = 1; i < mLights.size() + 1; ++i)
	{
		mMainLight.Lights[i] = mLights[i - 1].light;
		if (i == MAX_LIGHTS - 1)
			break;
	}

	mMainLight.Lights[0] = mDirectionalLight.light;
	mMainLight.numLights = (mLights.size() + 1 < MAX_LIGHTS) ? (int)mLights.size() + 1 : MAX_LIGHTS;

	mLightCB->CopyData(0, mMainLight);
}

void InGameScene::UpdateCameraConstant(int idx, Camera* camera)
{
	// 카메라로부터 상수를 받는다.
	mCameraCB->CopyData(idx, camera->GetConstants());
}

void InGameScene::UpdateVolumetricConstant()
{
	VolumetricConstants volumeConst;

	volumeConst.InvProj = Matrix4x4::Transpose(mCurrentCamera->GetInverseProj());
	volumeConst.View = Matrix4x4::Transpose(mCurrentCamera->GetView());

	for (int i = 0; i < mShadowMapRenderer->GetMapCount(); ++i)
	{
		volumeConst.ShadowTransform[i] = Matrix4x4::Transpose(Matrix4x4::Multiply(mShadowMapRenderer->GetView(i), mShadowMapRenderer->GetProj(i)));
		volumeConst.frstumSplit[i] = mShadowMapRenderer->GetSplit(i + 1);
	}

	volumeConst.numLights = (mLights.size() + 1 < MAX_LIGHTS) ? (int)mLights.size() + 1 : MAX_LIGHTS;

	volumeConst.Lights[0] = mDirectionalLight.volumetric;
	for (int i = 1; i < volumeConst.numLights; ++i)
	{
		if (mLights[i].volumetric.Type == SPOT_LIGHT || mLights[i].volumetric.Type == DIRECTIONAL_LIGHT)
		{
			volumeConst.Lights[i] = mLights[i - 1].volumetric;
		}
	}

	mVolumetricCB->CopyData(0, volumeConst);
}

void InGameScene::UpdateConstants(const GameTimer& timer)
{
	UpdateCameraConstant(0, mCurrentCamera);
	UpdateLightConstants();
	UpdateVolumetricConstant();

	GameInfoConstants gameInfo{};
	gameInfo.RandFloat4 = XMFLOAT4(
		Math::RandFloat(-1.0f, 1.0f),
		Math::RandFloat(-1.0f, 1.0f),
		Math::RandFloat(-1.0f, 1.0f),
		Math::RandFloat(0.0f, 1.0f));
	gameInfo.CurrentTime = timer.CurrentTime();
	gameInfo.ElapsedTime = timer.ElapsedTime();

	mGameInfoCB->CopyData(0, gameInfo);

	for (const auto& [layer, pso] : mPipelines)
	{
		if (layer != Layer::Transparent && layer != Layer::Instancing)
			pso->UpdateConstants(mMainCamera.get(), DrawType::Common);
	}
}

void InGameScene::UpdateInstancingPipelines(Camera* cam, DrawType type, bool culling)
{
	for (const auto& [layer, pso] : mPipelines)
	{
		if (layer == Layer::Transparent || layer == Layer::Instancing)
			pso->UpdateConstants(cam, type, culling);
	}
}

void InGameScene::UpdateDynamicsWorld()
{
	auto terrain = static_pointer_cast<TerrainObject>(mPipelines[Layer::Terrain]->GetRenderObjects()[0]);
	auto [blockWidth, blockDepth] = terrain->GetBlockSize();

	auto rigidBodies = terrain->GetTerrainRigidBodies();

	for (auto o : rigidBodies)
	{
		mDynamicsWorld->removeRigidBody(o);
	}

	for (int i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		if (!body->isStaticObject())
		{
			auto pos = body->getCenterOfMassPosition();

			int xIndex = (int)(pos.x() / blockWidth);
			int zIndex = (int)(pos.z() / blockDepth);

			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < 3; ++k)
				{
					int idx = (xIndex - 1 + j) + (zIndex - 1 + k) * (int)(terrain->GetWidth() / blockWidth);

					if (idx >= 0 && idx < rigidBodies.size())
					{
						if (!rigidBodies[idx]->isInWorld())
						{
							mDynamicsWorld->addRigidBody(rigidBodies[idx]);
						}
					}
				}
			}
		}
	}
}

void InGameScene::SetGraphicsCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex)
{
	cmdList->SetGraphicsRootConstantBufferView(0, mCameraCB->GetGPUVirtualAddress(cameraCBIndex));
	cmdList->SetGraphicsRootConstantBufferView(1, mLightCB->GetGPUVirtualAddress(0));
	cmdList->SetGraphicsRootConstantBufferView(2, mGameInfoCB->GetGPUVirtualAddress(0));
}

void InGameScene::SetComputeCBV(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetComputeRootConstantBufferView(3, mVolumetricCB->GetGPUVirtualAddress(0));
}

void InGameScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame)
{
	const XMFLOAT4& velocity = { 0.0f, 0.0f, 0.0f, 0.0f };
	cmdList->ClearRenderTargetView(mMsaaVelocityMapRtvHandle, (FLOAT*)&velocity, 0, nullptr);

	const XMFLOAT4& color = { 0.0f, 0.0f, 0.0f, 0.0f };
	cmdList->ClearRenderTargetView(mMsaaRtvHandle, (FLOAT*)&color, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE pd3dAllRtvCPUHandles[2] = { mMsaaRtvHandle, mMsaaVelocityMapRtvHandle };

	cmdList->OMSetRenderTargets(2, pd3dAllRtvCPUHandles, FALSE, &depthStencilView);

#ifdef FRUSTUM_CULLING
	UpdateInstancingPipelines(mMainCamera.get(), DrawType::Instancing);
#endif

#ifndef FRUSTUM_CULLING
	UpdateInstancingPipelines(mMainCamera.get(), DrawType::Instancing, false);
#endif

	cmdList->SetGraphicsRootSignature(mRootSignature.Get());
	RenderPipelines(cmdList, 0);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mMsaaTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

	cmdList->ResolveSubresource(backBuffer, 0, mMsaaTarget.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		backBuffer, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mMsaaTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	if (mMotionBlurEnable)
	{
		mPostProcessingPipelines[Layer::MotionBlur]->SetInput(cmdList, mMsaaVelocityMap.Get(), 0, true);
		mPostProcessingPipelines[Layer::MotionBlur]->SetInput(cmdList, backBuffer, 1);

		mPostProcessingPipelines[Layer::MotionBlur]->Dispatch(cmdList);

		mPostProcessingPipelines[Layer::MotionBlur]->CopyMapToRT(cmdList, backBuffer);
	}

	if (mBloomEnable)
	{
		mPostProcessingPipelines[Layer::Bloom]->SetInput(cmdList, backBuffer, 1);

		mPostProcessingPipelines[Layer::Bloom]->Dispatch(cmdList);

		mPostProcessingPipelines[Layer::Bloom]->CopyMapToRT(cmdList, backBuffer);
	}

	if (mVolumetricEnable)
	{
		SetComputeCBV(cmdList);
		mShadowMapRenderer->SetShadowMapComputeSRV(cmdList, 4);

		mPostProcessingPipelines[Layer::VolumetricScattering]->SetInput(cmdList, backBuffer, 0);
		mPostProcessingPipelines[Layer::VolumetricScattering]->SetInput(cmdList, depthBuffer, 1, true);

		mPostProcessingPipelines[Layer::VolumetricScattering]->Dispatch(cmdList);

		mPostProcessingPipelines[Layer::VolumetricScattering]->CopyMapToRT(cmdList, backBuffer);
	}

	mpUI.get()->Draw(nFrame);
}

void InGameScene::RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex, bool cubeMapping)
{
	SetGraphicsCBV(cmdList, cameraCBIndex);
	mShadowMapRenderer->SetShadowMapGraphicsSRV(cmdList, 6);

	for (const auto& [layer, pso] : mPipelines)
	{
		/*if (layer != Layer::Terrain && layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, mCurrentCamera->GetWorldFrustum(), true, (bool)mLODSet);
		else if (layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, mCurrentCamera->GetWorldFrustum(), false, (bool)mLODSet);
		else*/
		DrawType type = cubeMapping ? DrawType::CubeMapping : DrawType::Instancing;

		if (pso->GetRenderObjects().size() > 0)
		{
			if (layer == Layer::CheckPoint && !cubeMapping)
			{
				if (mCheckPointEnable)
				{
					pso->SetAndDraw(cmdList, true, true, cubeMapping);
				}
				else
				{
					continue;
				}
			}
			else if (cubeMapping && (layer == Layer::Color || layer == Layer::DriftParticle || layer == Layer::MissileParticle || layer == Layer::CheckPoint))
			{
				continue;
			}
			else
				pso->SetAndDraw(cmdList, (bool)mLODSet, true, cubeMapping, type);
		}
	}
}

void InGameScene::UpdateMissileObject()
{
	bool flag = false;
	for (int i = 0; i < mMissileObjects.size(); i++)
	{
		auto missile = mMissileObjects[i].get();
		if (missile == nullptr) continue;

		switch (missile->GetUpdateFlag())
		{
		case UPDATE_FLAG::CREATE:
		{
			flag = true;
			missile->SetActive(true);
			mPipelines[Layer::Default]->AppendObject(mMissileObjects[i]);
			missile->SetUpdateFlag(UPDATE_FLAG::NONE);

			auto particle = missile->GetParticle();
			particle->SetParticleEnable(true);

			break;
		}
		case UPDATE_FLAG::REMOVE:
		{
			flag = true;
			missile->SetActive(false);
			missile->RemoveObject(*mDynamicsWorld, *mPipelines[Layer::Default]);
			missile->SetUpdateFlag(UPDATE_FLAG::NONE);

			auto particle = missile->GetParticle();
			particle->SetParticleEnable(false);

			break;
		}
		case UPDATE_FLAG::NONE:
			break;
		}
	}
	if (flag) mPipelines[Layer::Default]->ResetPipeline(mDevice);
}

void InGameScene::UpdatePlayerObjects()
{
	bool removed_flag = false;
	for (int i = 0; i < mPlayerObjects.size(); i++)
	{
		auto player = mPlayerObjects[i].get();
		if (player == nullptr) continue;

		switch (player->GetUpdateFlag())
		{
		case UPDATE_FLAG::CREATE:
		{
			player->SetUpdateFlag(UPDATE_FLAG::NONE);
			break;
		}
		case UPDATE_FLAG::REMOVE:
		{
			removed_flag = true;
			if (mMissileObjects[i]) mMissileObjects[i]->SetUpdateFlag(UPDATE_FLAG::REMOVE);
			player->RemoveObject(*mDynamicsWorld, *mPipelines[Layer::Color]);
			player->SetUpdateFlag(UPDATE_FLAG::NONE);
			break;
		}
		case UPDATE_FLAG::NONE:
			break;
		}
	}
	if (removed_flag) mPipelines[Layer::Color]->ResetPipeline(mDevice);
}

void InGameScene::LoadWorldMap(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, const std::string& path)
{
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "r");

#ifdef STANDALONE	
	btCompoundShape* compound = new btCompoundShape();
#endif

	char buf[250];
	while (fgets(buf, 250, file))
	{
		std::stringstream ss(buf);

		std::string objName;
		ss >> objName;

		XMFLOAT3 pos;
		ss >> pos.x >> pos.y >> pos.z;

		XMFLOAT4 quaternion;
		ss >> quaternion.x >> quaternion.y >> quaternion.z >> quaternion.w;

		XMFLOAT3 scale;
		ss >> scale.x >> scale.y >> scale.z;

		auto tmpstr = std::string("Models\\") + objName + std::string(".obj");

		auto transparentpath = tmpstr;
		transparentpath.replace(tmpstr.find(".obj"), 4, "_Transparent.obj");

		wstring objPath;
		objPath.assign(tmpstr.begin(), tmpstr.end());

		wstring transparentObjPath;
		transparentObjPath.assign(transparentpath.begin(), transparentpath.end());

		auto obj = make_shared<StaticObject>();
		if (static_cast<InstancingPipeline*>(mPipelines[Layer::Instancing].get())->mInstancingCount[objName] == 0)
		{
#ifdef STANDALONE
			obj->LoadModel(mDevice.Get(), cmdList, objPath, true);
#else
			obj->LoadModel(mDevice, cmdList, objPath, false);
#endif
			mMeshList[objName] = obj->GetMeshes();
			mOOBBList[objName] = obj->GetBoundingBox();
		}
		else
			obj->SetBoudingBox(mOOBBList[objName]);

		btTransform btLocalTransform;
		btLocalTransform.setIdentity();
		btLocalTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
		btLocalTransform.setRotation(btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w));

#ifdef STANDALONE
		auto& meshes = mMeshList[objName];
		for (auto i = meshes.begin(); i < meshes.end(); ++i)
		{
			if (i->get()->GetMeshShape())
			{
				i->get()->GetMeshShape()->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
				compound->addChildShape(btLocalTransform, i->get()->GetMeshShape().get());
			}
		}
#endif

		wstring convexObjPath;
		tmpstr.erase(tmpstr.end() - 4, tmpstr.end());
		convexObjPath.assign(tmpstr.begin(), tmpstr.end());

		obj->LoadConvexHullShape(convexObjPath + L"_Convex_Hull.obj", physics);
		obj->SetQuaternion(quaternion);
		obj->SetPosition(pos);
		obj->Scale(scale);
		obj->SetName(objName);

		obj->Update(0, 0);
		obj->UpdateInverseWorld();
		mPipelines[Layer::Instancing]->AppendObject(obj);
		static_cast<InstancingPipeline*>(mPipelines[Layer::Instancing].get())->mInstancingCount[objName]++;

		if (_access(transparentpath.c_str(), 0) != -1)
		{
			auto transparentObj = make_shared<StaticObject>();

#ifdef STANDALONE
			transparentObj->LoadModel(mDevice.Get(), cmdList, transparentObjPath, true);
#else
			transparentObj->LoadModel(mDevice, cmdList, transparentObjPath, false);
#endif

			mMeshList[objName + "_Transparent"] = transparentObj->GetMeshes();
			mOOBBList[objName + "_Transparent"] = transparentObj->GetBoundingBox();

#ifdef STANDALONE
			auto& transparentMeshes = transparentObj->GetMeshes();
			for (auto i = transparentMeshes.begin(); i < transparentMeshes.end(); ++i)
			{
				if (i->get()->GetMeshShape())
				{
					compound->addChildShape(btLocalTransform, i->get()->GetMeshShape().get());
				}
			}
#endif
			transparentObj->SetQuaternion(quaternion);
			transparentObj->SetPosition(pos);
			transparentObj->Scale(scale);
			transparentObj->SetName(objName + "_Transparent");

			transparentObj->Update(0, 0);
			transparentObj->UpdateInverseWorld();

			mPipelines[Layer::Transparent]->AppendObject(transparentObj);
			static_cast<InstancingPipeline*>(mPipelines[Layer::Transparent].get())->mInstancingCount[objName]++;
		}
	}

#ifdef STANDALONE
	btTransform btObjectTransform;
	btObjectTransform.setIdentity();
	btObjectTransform.setOrigin(btVector3(0, 0, 0));
	mTrackRigidBody = physics->CreateRigidBody(0.0f, btObjectTransform, compound);
#endif
	//mTrackRigidBody = physics->CreateRigidBody(0.0f, btObjectTransform, compound);

	fclose(file);
}

void InGameScene::LoadCheckPoint(ID3D12GraphicsCommandList* cmdList, const std::wstring& path)
{
	std::ifstream in_file{ path };
	std::string info;

	std::getline(in_file, info);
	std::stringstream extss(info);

	XMFLOAT3 extent;

	extss >> extent.x >> extent.y >> extent.z;

	while (std::getline(in_file, info))
	{
		std::stringstream ss(info);

		XMFLOAT3 pos;
		ss >> pos.x >> pos.y >> pos.z;

		XMFLOAT4 quaternion;
		ss >> quaternion.x >> quaternion.y >> quaternion.z >> quaternion.w;

		auto obj = make_shared<StaticObject>();

		std::shared_ptr<BoxMesh> mesh = std::make_shared<BoxMesh>(mDevice, cmdList, extent.x * 2, extent.y * 2, extent.z * 2);
		mesh->SetSrvIndex(0);
		obj->LoadTexture(mDevice, cmdList, L"Resources\\tile.dds", D3D12_SRV_DIMENSION_TEXTURE2D);
		obj->SetMesh(mesh);
		obj->SetPosition(pos);
		obj->SetQuaternion(quaternion);

		mPipelines[Layer::CheckPoint]->AppendObject(obj);
	}

	auto objects = mPipelines[Layer::Instancing]->GetRenderObjects();
	for (int i = 0; i < objects.size(); ++i)
	{
		auto obj = make_shared<StaticObject>();
		auto oobb = objects[i]->GetBoundingBox();
		oobb.Transform(oobb, XMLoadFloat4x4(&objects[i]->GetWorld()));

		std::shared_ptr<BoxMesh> mesh = std::make_shared<BoxMesh>(mDevice, cmdList, oobb.Extents.x * 2, oobb.Extents.y * 2, oobb.Extents.z * 2);
		mesh->SetSrvIndex(0);
		obj->LoadTexture(mDevice, cmdList, L"Resources\\tile.dds", D3D12_SRV_DIMENSION_TEXTURE2D);
		obj->SetMesh(mesh);
		obj->SetPosition(oobb.Center);
		obj->SetQuaternion(oobb.Orientation);

		mPipelines[Layer::CheckPoint]->AppendObject(obj);
	}
}


void InGameScene::WriteOOBBList()
{
	const std::string& path = "Map\\OOBBList.txt";
	std::ofstream out_file{ path };

	for (const auto& [name, oobb] : mOOBBList)
	{
		out_file << name << " " << mMeshList[name].size() << " " << oobb.Center.x << " " << oobb.Center.y << " " << oobb.Center.z << " " << oobb.Extents.x << " " << oobb.Extents.y << " " << oobb.Extents.z << "\n";
	}
}

void InGameScene::LoadLights(ID3D12GraphicsCommandList* cmdList, const std::wstring& path)
{
	std::ifstream in_file{ path };
	std::string info;

	while (std::getline(in_file, info))
	{
		std::stringstream ss(info);

		string type;
		ss >> type;

		if (type == "S")
		{
			XMFLOAT3 pos;
			ss >> pos.x >> pos.y >> pos.z;

			XMFLOAT3 direction;
			ss >> direction.x >> direction.y >> direction.z;

			LightBundle bundle;
			LightInfo l;

			l.SetInfo(
				XMFLOAT3(0.6f, 0.6f, 0.6f),
				pos,
				direction,
				10.0f, 20.0f, 10.0f,
				0.0f, SPOT_LIGHT);;

			bundle.light = l;

			VolumetricInfo v;

			v.Direction = direction;
			v.Position = pos;
			v.Range = 30.0f;
			v.VolumetricStrength = 0.5f;
			v.outerCosine = cos(7.0f);
			v.innerCosine = cos(6.0f);
			v.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
			v.Type = SPOT_LIGHT;

			bundle.volumetric = v;

			mLights.push_back(bundle);
		}
		else if (type == "P_Tunnel")
		{
			XMFLOAT3 pos;
			ss >> pos.x >> pos.y >> pos.z;

			XMFLOAT3 direction;
			ss >> direction.x >> direction.y >> direction.z;

			LightBundle bundle;
			LightInfo l;

			l.SetInfo(
				XMFLOAT3(1.0f, 0.5f, 0.0f),
				pos,
				XMFLOAT3{ 0.0f, 0.0f, 0.0f },
				10.0f, 20.0f, 0.0f,
				0.0f, POINT_LIGHT);;

			bundle.light = l;

			VolumetricInfo v;

			v.Direction = XMFLOAT3{ 0.0f, -1.0f, 0.0f };
			v.Position = pos;
			v.Range = 30.0f;
			v.VolumetricStrength = 0.2f;
			v.outerCosine = cos(10.0f);
			v.innerCosine = cos(9.0f);
			v.Color = XMFLOAT3(1.0f, 0.5f, 0.0f);
			v.Type = SPOT_LIGHT;

			bundle.volumetric = v;

			mLights.push_back(bundle);
		}
		else if (type == "P_Deco")
		{

		}
	}
}

void InGameScene::SetSound()
{
	std::vector<std::string> SoundFiles;
	SoundFiles.push_back("Sound/IngameSound/BGM1.wav");
	SoundFiles.push_back("Sound/IngameSound/edmBGM2.mp3");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/DriftLoop.wav");
	SoundFiles.push_back("Sound/IngameSound/DrivingReverse.mp3");
	SoundFiles.push_back("Sound/IngameSound/BrakeSkid.wav");
	SoundFiles.push_back("Sound/IngameSound/BikeBrake.wav");

	SoundFiles.push_back("Sound/IngameSound/CarBoost4.wav");
	SoundFiles.push_back("Sound/IngameSound/BoosterBack.wav");
	SoundFiles.push_back("Sound/IngameSound/Missile.wav");
	SoundFiles.push_back("Sound/IngameSound/MissileExplosion.wav");

	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");
	SoundFiles.push_back("Sound/IngameSound/CarEngine2.wav");




	std::vector<FMOD_MODE> modes;
	modes.push_back(FMOD_LOOP_NORMAL);
	modes.push_back(FMOD_LOOP_NORMAL);
	modes.push_back(FMOD_LOOP_NORMAL);
	modes.push_back(FMOD_LOOP_NORMAL);
	modes.push_back(FMOD_DEFAULT);
	modes.push_back(FMOD_DEFAULT);
	modes.push_back(FMOD_DEFAULT);

	modes.push_back(FMOD_DEFAULT);
	modes.push_back(FMOD_DEFAULT);
	modes.push_back(FMOD_DEFAULT | FMOD_3D);
	modes.push_back(FMOD_DEFAULT | FMOD_3D);

	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);
	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);
	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);
	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);
	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);
	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);
	modes.push_back(FMOD_LOOP_NORMAL | FMOD_3D);





	GetSound().InitSound(SoundFiles, modes);
}

void InGameScene::BuildListener(const XMFLOAT3& CameraPos, const XMFLOAT3& CameraLook, const XMFLOAT3& CameraUp)
{
	//mListener Set
	FMOD_VECTOR ListenerPos{}, ListenerForward{}, ListenerUp{}, ListenerVelocity{};

	ListenerPos.x = CameraPos.x;
	ListenerPos.y = CameraPos.y;
	ListenerPos.z = CameraPos.z;

	ListenerForward.x = CameraLook.x;
	ListenerForward.y = CameraLook.y;
	ListenerForward.z = CameraLook.z;

	ListenerUp.x = CameraUp.x;
	ListenerUp.y = CameraUp.y;
	ListenerUp.z = CameraUp.z;

	ListenerVelocity = { 0.0f, 0.0f, 0.0f };

	FMOD_System_Set3DListenerAttributes(GetSound().GetSystem(), 0, &ListenerPos, &ListenerForward, &ListenerUp, &ListenerVelocity);
	FMOD_System_Set3DSettings(GetSound().GetSystem(), 1.0f, 50.0f, 1.0f);



}

void InGameScene::Update3DSound()
{

	auto& sound = GetSound();
	FMOD_VECTOR Pos{}, Vel{}, Forward{}, Up{};

	Pos.x = mCurrentCamera->GetPosition().x;
	Pos.y = mCurrentCamera->GetPosition().y;
	Pos.z = mCurrentCamera->GetPosition().z;

	Forward.x = mCurrentCamera->GetLook().x;
	Forward.y = mCurrentCamera->GetLook().y;
	Forward.z = mCurrentCamera->GetLook().z;

	Up.x = mCurrentCamera->GetUp().x;
	Up.y = mCurrentCamera->GetUp().y;
	Up.z = mCurrentCamera->GetUp().z;

	Vel.x = mPlayer->GetLinearVelocity().GetXMFloat3().x;
	Vel.y = mPlayer->GetLinearVelocity().GetXMFloat3().y;
	Vel.z = mPlayer->GetLinearVelocity().GetXMFloat3().z;

	FMOD_System_Set3DListenerAttributes(sound.GetSystem(), 0, &Pos, &Vel, &Forward, &Up);

	FMOD_VECTOR SoundPos{}, SoundVel{};
	for (int i = 0; i < mMissileObjects.size(); ++i)
	{
		auto Missile = mMissileObjects[i].get();
		if (Missile == nullptr)
			continue;
		auto& missilePos = Missile->GetPosition();
		auto& missileVel = Missile->GetLinearVelocity();
		SoundPos.x = missilePos.x;
		SoundPos.y = missilePos.y;
		SoundPos.z = missilePos.z;
		SoundVel.x = missileVel.GetXMFloat3().x;
		SoundVel.y = missileVel.GetXMFloat3().y;
		SoundVel.z = missileVel.GetXMFloat3().z;
	}
	sound.Set3DPos(static_cast<int>(IngameUI_SOUND_TRACK::MISSILE), SoundPos, SoundVel);

	//Player 3D Sound Update
	
	for (int i=0;i<mPlayerObjects.size();++i)
	{
		if (!mPlayerObjects[i])
			continue;
		
		SoundPos.x = mPlayerObjects[i].get()->GetPosition().x;
		SoundPos.y = mPlayerObjects[i].get()->GetPosition().y;
		SoundPos.z = mPlayerObjects[i].get()->GetPosition().z;
		
		SoundVel.x = 0.0f;
		SoundVel.y = 0.0f;
		SoundVel.z = 0.0f;
		if (mPlayerObjects[i].get() == mPlayer)
		{
			SoundPos.x = (mCurrentCamera->GetPosition().x + mPlayerObjects[i].get()->GetPosition().x) / 2.0f;
			SoundPos.y = (mCurrentCamera->GetPosition().y + mPlayerObjects[i].get()->GetPosition().y) / 2.0f;
			SoundPos.z = (mCurrentCamera->GetPosition().z + mPlayerObjects[i].get()->GetPosition().z) / 2.0f;
			sound.Update3DSoundForPlayer(static_cast<int>(IngameUI_SOUND_TRACK::PLAYER1) + i, SoundPos, SoundVel, mPlayerObjects[i].get()->GetCurrentSpeed());
		}
		else 
		{
			sound.Update3DSoundForOtherPlayers(static_cast<int>(IngameUI_SOUND_TRACK::PLAYER1) + i, SoundPos, SoundVel, mPlayerObjects[i].get()->GetCurrentSpeed());
		}
	}


}