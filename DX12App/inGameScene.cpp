#include "stdafx.h"
#include "Mesh.h"
#include "shadowMapRenderer.h"
#include "LoginUI.h"
#include "InGameUI.h"
#include "NetLib/NetModule.h"
#include "inGameScene.h"

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
	mKeyMap['Z'] = false;
	mKeyMap['X'] = false;

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
	if(mMainCamera)
		mMainCamera->SetLens(aspect);
	if (mDirectorCamera)
		mDirectorCamera->SetLens(aspect);
}

void InGameScene::BuildObjects(
	ComPtr<ID3D12Device> device, 
	ID3D12GraphicsCommandList* cmdList, 
	ID3D12CommandQueue* cmdQueue,
	UINT nFrame, ID3D12Resource** backBuffer, 
	float Width, float Height, float aspect,
	const shared_ptr<BulletWrapper>& physics)
{
	mDevice = device;

	mMainCamera = make_unique<Camera>();
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 4000.0f);
	mMainCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3( 0.0f,0.0f,0.0f ), XMFLOAT3( 0.0f,1.0f,0.0f ));
	mMainCamera->SetPosition(0.0f, 0.0f, 0.0f);
	mMainCamera->Move(mMainCamera->GetLook(), -mCameraRadius);

	mDirectorCamera = make_unique<Camera>();
	mDirectorCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 20000.0f);
	mDirectorCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	mDirectorCamera->SetPosition(500.0f, 50.0f, 500.0f);

	mCurrentCamera = mDirectorCamera.get();

	mMainLight.Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mMainLight.Lights[0].SetInfo(
		XMFLOAT3(0.6f, 0.6f, 0.6f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, -1.0f),
		0.0f, 0.0f, 0.0f,
		3000.0f, DIRECTIONAL_LIGHT);
	mMainLight.Lights[1].SetInfo(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, -1.0f),
		0.0f, 0.0f, 0.0f,
		3000.0f, DIRECTIONAL_LIGHT);
	mMainLight.Lights[2].SetInfo(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, 1.0f),
		0.0f, 0.0f, 0.0f,
		3000.0f, DIRECTIONAL_LIGHT);

	BuildRootSignature();
	BuildComputeRootSignature();
	BuildShadersAndPSOs(cmdList);
	BuildGameObjects(cmdList, physics);
	BuildConstantBuffers();
	BuildDescriptorHeap();

	mpUI = std::make_unique<InGameUI>(nFrame, mDevice, cmdQueue);
	mpUI.get()->PreDraw(backBuffer, static_cast<UINT>(Width), static_cast<UINT>(Height));

	// Let server know that loading sequence is done.
#ifndef STANDALONE
	mNetPtr->Client()->SendLoadSequenceDone(mNetPtr->GetRoomID());
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
	parameters[8] = Extension::Constants(4, 5, D3D12_SHADER_VISIBILITY_ALL);                                 // E.T.C - 기타 바로바로 올려야 할 필요가 있는 쉐이더 상수들
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
	D3D12_DESCRIPTOR_RANGE descRanges[2];
	descRanges[0] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
	descRanges[1] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	D3D12_ROOT_PARAMETER parameters[3];
	parameters[0] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);    // Inputs
	parameters[1] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_ALL);    // Output																   
	parameters[2] = Extension::Constants(2, 0, D3D12_SHADER_VISIBILITY_ALL);					   // 32bit Constant
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = Extension::RootSignatureDesc(_countof(parameters), parameters,
		0, nullptr , D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	auto defaultShader = make_unique<DefaultShader>(L"Shaders\\default.hlsl");
	auto instancingShader = make_unique<DefaultShader>(L"Shaders\\Instancing.hlsl");
	auto colorShader = make_unique<DefaultShader>(L"Shaders\\color.hlsl");
	auto terrainShader = make_unique<TerrainShader>(L"Shaders\\terrain.hlsl");
	auto motionBlurShader = make_unique<ComputeShader>(L"Shaders\\motionBlur.hlsl");

	mPipelines[Layer::Default] = make_unique<Pipeline>();
	mPipelines[Layer::Terrain] = make_unique<Pipeline>();
	mPipelines[Layer::SkyBox] = make_unique<SkyboxPipeline>(mDevice.Get(), cmdList);
	mPipelines[Layer::Instancing] = make_unique<InstancingPipeline>();
	mPipelines[Layer::Color] = make_unique<Pipeline>();
	mPipelines[Layer::Transparent] = make_unique<InstancingPipeline>();

	mShadowMapRenderer = make_unique<ShadowMapRenderer>(mDevice.Get(), 5000, 5000, 3, mCurrentCamera);

	if (mMsaa4xEnable)
	{
		for (const auto& [_, pso] : mPipelines)
		{
			if (pso)
				pso->SetMsaa(mMsaa4xEnable, mMsaa4xQualityLevels);
		}
	}

	mPipelines[Layer::Default]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), defaultShader.get());

	mPipelines[Layer::Terrain]->SetWiredFrame(true);
	mPipelines[Layer::Terrain]->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
	mPipelines[Layer::Terrain]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), terrainShader.get());

	mPipelines[Layer::Color]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), colorShader.get());
	
	mPipelines[Layer::Instancing]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), instancingShader.get());

	mPipelines[Layer::SkyBox]->BuildPipeline(mDevice.Get(), mRootSignature.Get());

	mPipelines[Layer::Transparent]->SetAlphaBlending();
	mPipelines[Layer::Transparent]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), instancingShader.get());

	mPostProcessingPipelines[Layer::MotionBlur] = make_unique<ComputePipeline>(mDevice.Get());
	mPostProcessingPipelines[Layer::MotionBlur]->BuildPipeline(mDevice.Get(), mComputeRootSignature.Get(), motionBlurShader.get());

	//mShadowMapRenderer->AppendTargetPipeline(Layer::Default, mPipelines[Layer::Default].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Color, mPipelines[Layer::Color].get());
	//mShadowMapRenderer->AppendTargetPipeline(Layer::Terrain, mPipelines[Layer::Terrain].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Instancing, mPipelines[Layer::Instancing].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Transparent, mPipelines[Layer::Transparent].get());
	mShadowMapRenderer->BuildPipeline(mDevice.Get(), mRootSignature.Get());

}

void InGameScene::BuildConstantBuffers()
{
	mLightCB = std::make_unique<ConstantBuffer<LightConstants>>(mDevice.Get(), 2);
	mCameraCB = std::make_unique<ConstantBuffer<CameraConstants>>(mDevice.Get(), 10); // 메인 카메라 1개, 그림자 매핑 카메라 3개, 다이나믹 큐브매핑 카메라 6개
	mGameInfoCB = std::make_unique<ConstantBuffer<GameInfoConstants>>(mDevice.Get(), 1);

	for (const auto& [_, pso] : mPipelines)
	{
		if(pso)
			pso->BuildConstantBuffer(mDevice.Get());
	}
}

void InGameScene::BuildDescriptorHeap()
{
	CreateVelocityMapDescriptorHeaps();
	CreateVelocityMapViews();

	CreateMsaaDescriptorHeaps();
	CreateMsaaViews();

	mShadowMapRenderer->BuildDescriptorHeap(mDevice.Get(), 3, 4, 5);
	for (const auto& [_, pso] : mPipelines)
		pso->BuildDescriptorHeap(mDevice.Get(), 3, 4, 5);
}

void InGameScene::CreateVelocityMapViews()
{
	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R32G32B32A32_FLOAT, {0.0f,0.0f,0.0f,0.0f} };

	mMsaaVelocityMap = CreateTexture2DResource(
		mDevice.Get(), gFrameWidth, gFrameHeight, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, 4, mMsaa4xQualityLevels - 1);

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
		mDevice.Get(), gFrameWidth, gFrameHeight, 1, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, 4, mMsaa4xQualityLevels - 1);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mMsaaRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	mDevice->CreateRenderTargetView(mMsaaTarget.Get(), &rtvDesc, rtvHandle);
	mMsaaRtvHandle = rtvHandle;
}

void InGameScene::BuildGameObjects(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics)
{
	mDynamicsWorld = physics->GetDynamicsWorld();

	mMeshList["Missile"].push_back(std::make_shared<BoxMesh>(mDevice.Get(), cmdList, 5.f, 5.f, 5.f));

	// 지형 스케일에는 정수를 넣는 것을 권장
	/*auto terrain = make_shared<TerrainObject>(1024, 1024, XMFLOAT3(8.0f, 1.0f, 8.0f));
	terrain->BuildHeightMap(L"Resources\\PlaneMap.raw");
	terrain->BuildTerrainMesh(mDevice.Get(), cmdList, physics, 129, 129);
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\terrainTexture.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\rocky.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\road.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\heightmap.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\normalmap.dds");
	mPipelines[Layer::Terrain]->AppendObject(terrain);*/
	//physics->SetTerrainRigidBodies(terrain->GetTerrainRigidBodies());
	LoadWorldMap(cmdList, physics, L"Map\\MapData.tmap");

#ifdef STANDALONE
	BuildCarObject({ -3200.0f, 10.0f, 1500.0f }, 4, true, cmdList, physics, 0);
#else
	const auto& players = mNetPtr->GetPlayersInfo();
	for (int i = 0; const PlayerInfo& info : players)
	{
		if (info.Empty == false)
		{
			bool isPlayer = (i == mNetPtr->GetPlayerIndex()) ? true : false;
			BuildCarObject(info.StartPosition, info.Color, isPlayer, cmdList, physics, i);
			BuildMissileObject(cmdList, info.StartPosition, i);
		}
		i++;
	}
#endif
	float aspect = mMainCamera->GetAspect();
	mMainCamera.reset(mPlayer->ChangeCameraMode((int)CameraMode::THIRD_PERSON_CAMERA));
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 4000.0f);
	mCurrentCamera = mMainCamera.get();
}

void InGameScene::BuildCarObject(
	const XMFLOAT3& position,
	char color,
	bool isPlayer,
	ID3D12GraphicsCommandList* cmdList, 
	const std::shared_ptr<BulletWrapper>& physics,
	UINT netID)
{
	auto carObj = make_shared<PhysicsPlayer>(netID);
	carObj->SetPosition(position);

	if (mMeshList["Car_Body.obj"].empty())
		mMeshList["Car_Body.obj"] = carObj->LoadModel(mDevice.Get(), cmdList, L"Models\\Car_Body.obj");
	else
		carObj->CopyMeshes(mMeshList["Car_Body.obj"]);

	carObj->SetDiffuse("Car_Texture", mColorMap[(int)color]);
	for (int i = 0; i < 4; ++i)
	{
		auto wheelObj = make_shared<WheelObject>(*carObj.get());

		float x = 7.6f, y = -1.2f, z = 11.0f;
		if (i >= 2)
		{
			z *= -1.0f;
		}
		if (i % 2 == 0)
		{
			x *= -1.0f;
		}
		wheelObj->SetLocalOffset({ x, y, z });


		if (i % 2 == 0)
		{
			if (mMeshList["Car_Wheel_L.obj"].empty())
				mMeshList["Car_Wheel_L.obj"] = wheelObj->LoadModel(mDevice.Get(), cmdList, L"Models\\Car_Wheel_L.obj");
			else
				wheelObj->CopyMeshes(mMeshList["Car_Wheel_L.obj"]);
		}
		else
		{
			if (mMeshList["Car_Wheel_R.obj"].empty())
				mMeshList["Car_Wheel_R.obj"] = wheelObj->LoadModel(mDevice.Get(), cmdList, L"Models\\Car_Wheel_R.obj");
			else
				wheelObj->CopyMeshes(mMeshList["Car_Wheel_R.obj"]);
		}

		carObj->SetWheel(wheelObj, i);
		mPipelines[Layer::Color]->AppendObject(wheelObj);
	}

#ifdef STANDALONE
	carObj->BuildRigidBody(physics);
#endif
	carObj->BuildDsvRtvView(mDevice.Get());

	if (isPlayer) mPlayer = carObj.get();
	mPipelines[Layer::Color]->AppendObject(carObj);
	mPlayerObjects[netID] = std::move(carObj);	
}

void InGameScene::BuildMissileObject(
	ID3D12GraphicsCommandList* cmdList, 
	const XMFLOAT3& position, int idx)
{
	mMissileObjects[idx] = std::make_shared<MissileObject>(position);
	mMissileObjects[idx]->SetMeshes(mMeshList["Missile"]);
	mMissileObjects[idx]->LoadTexture(mDevice.Get(), cmdList, L"Resources\\tile.dds");
}

void InGameScene::PreRender(ID3D12GraphicsCommandList* cmdList, float elapsed)
{
	if (mShadowMapRenderer)
		mShadowMapRenderer->PreRender(cmdList, this);

	if (mCubemapInterval < 0.0f)
	{
		mCubemapInterval = 0.03f;
		mPlayer->PreDraw(cmdList, this, mCubemapDrawIndex);

		if (mCubemapDrawIndex < 5)
			mCubemapDrawIndex++;
		else
		{
			mPlayer->ChangeCurrentRenderTarget();
			mCubemapDrawIndex = 0;
		}
	}
	else
	{
		mCubemapInterval -= elapsed;
	}
	mPlayer->SetCubemapSrv(cmdList, 7);
}

bool InGameScene::ProcessPacket(std::byte* packet, char type, int bytes)
{
	switch (type)
	{
	case SC::START_SIGNAL:
	{
		SC::packet_start_signal* pck = reinterpret_cast<SC::packet_start_signal*>(packet);
		mGameStarted = true;
		// TODO: Start signal
		break;
	}
	case SC::REMOVE_PLAYER:
	{
		SC::packet_remove_player* pck = reinterpret_cast<SC::packet_remove_player*>(packet);
		mNetPtr->RemovePlayer(pck);

		auto player = mPlayerObjects[pck->player_idx];
		if (player)	player->SetUpdateFlag(UPDATE_FLAG::REMOVE);
		break;
	}
	case SC::TRANSFER_TIME:
	{
		SC::packet_transfer_time* pck = reinterpret_cast<SC::packet_transfer_time*>(packet);
		mNetPtr->SetLatency(pck->c_send_time);
		mNetPtr->Client()->ReturnSendTimeBack(pck->s_send_time);
		break;
	}
	case SC::PLAYER_TRANSFORM:
	{
		SC::packet_player_transform* pck = reinterpret_cast<SC::packet_player_transform*>(packet);
		auto player = mPlayerObjects[pck->player_idx];
		
		if (player)
		{
			if (player.get() == mPlayer) mNetPtr->SetUpdateRate();
			player->SetCorrectionTransform(pck, mNetPtr->GetLatency());
		}
		break;
	}
	case SC::MISSILE_TRANSFORM:
	{
		SC::packet_missile_transform* pck = reinterpret_cast<SC::packet_missile_transform*>(packet);
		auto& missile = mMissileObjects[pck->missile_idx];
		
		if (missile)
		{
			if (missile->IsActive() == false)
				missile->SetUpdateFlag(UPDATE_FLAG::CREATE);
			missile->SetCorrectionTransform(pck, mNetPtr->GetLatency());
		}
		break;
	}
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
	switch (uMsg)
	{
	case WM_KEYUP:
		if (wParam == 'G')
		{
			if (mCurrentCamera == mDirectorCamera.get())
				mCurrentCamera = mMainCamera.get();
			else
				mCurrentCamera = mDirectorCamera.get();
		}
		if(wParam == VK_END)
			SetSceneChangeFlag(SCENE_CHANGE_FLAG::POP);
		break;
	}
	mpUI->OnProcessKeyInput(uMsg, wParam, lParam);
}

void InGameScene::OnPreciseKeyInput(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, float elapsed)
{
	if (mHwnd != GetFocus()) return;

	if (GetAsyncKeyState('M') & 1)
	{
		mMotionBlurEnable = 1 - mMotionBlurEnable;
	}

	if (mCurrentCamera == mDirectorCamera.get())
	{
		const float dist = 500.0f;
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
	
	if (mMissileInterval < 0.0f)
	{
		if (GetAsyncKeyState('X') & 0x8000)
		{
			//mMissileInterval = 1.0f;
			//AppendMissileObject(cmdList, physics);
		}
	}
	else
	{
		mMissileInterval -= elapsed;
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
#endif
}

void InGameScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, const std::shared_ptr<BulletWrapper>& physics)
{
	float elapsed = timer.ElapsedTime();

	if(mGameStarted)
		physics->StepSimulation(elapsed);

	UpdateMissileObject();
	UpdatePlayerObjects();
	OnPreciseKeyInput(cmdList, physics, elapsed);

	UpdateLight(elapsed);
	mCurrentCamera->Update(elapsed);

	mShadowMapRenderer->UpdateSplitFrustum(mCurrentCamera);
	mShadowMapRenderer->UpdateDepthCamera(cmdList, mMainLight);

	for (const auto& [_, pso] : mPipelines)
		pso->Update(elapsed, mNetPtr->GetUpdateRate(), mCurrentCamera);
	
	UpdateConstants(timer);

	mpUI.get()->Update(timer.TotalTime(), mPlayer);
}

void InGameScene::UpdateLight(float elapsed)
{
}

void InGameScene::UpdateLightConstants()
{
	for(int i = 0; i < mShadowMapRenderer->GetMapCount(); ++i)
		mMainLight.ShadowTransform[i] = Matrix4x4::Transpose(mShadowMapRenderer->GetShadowTransform(i));

	mLightCB->CopyData(0, mMainLight);
}

void InGameScene::UpdateCameraConstant(int idx, Camera* camera)
{
	// 카메라로부터 상수를 받는다.
	mCameraCB->CopyData(idx, camera->GetConstants());
}

void InGameScene::UpdateConstants(const GameTimer& timer)
{
	UpdateCameraConstant(0, mCurrentCamera);
	UpdateLightConstants();	

	GameInfoConstants gameInfo{};
	gameInfo.RandFloat4 = XMFLOAT4(
		Math::RandFloat(-1.0f, 1.0f),
		Math::RandFloat(0.0f, 1.0f),
		Math::RandFloat(-1.0f, 1.0f),
		Math::RandFloat(1.0f, 5.0f));
	gameInfo.PlayerPosition = { 0.0f,0.0f,0.0f };
	gameInfo.CurrentTime = timer.CurrentTime();
	gameInfo.ElapsedTime = timer.ElapsedTime();

	mGameInfoCB->CopyData(0, gameInfo);
	
	for (const auto& [_, pso] : mPipelines)
		pso->UpdateConstants();
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

void InGameScene::SetCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex)
{
	cmdList->SetGraphicsRootConstantBufferView(0, mCameraCB->GetGPUVirtualAddress(cameraCBIndex));
	cmdList->SetGraphicsRootConstantBufferView(1, mLightCB->GetGPUVirtualAddress(0));
	cmdList->SetGraphicsRootConstantBufferView(2, mGameInfoCB->GetGPUVirtualAddress(0));
}

void InGameScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, UINT nFrame)
{
	const XMFLOAT4& velocity = { 0.0f, 0.0f, 0.0f, 0.0f };
	cmdList->ClearRenderTargetView(mMsaaVelocityMapRtvHandle, (FLOAT*)&velocity, 0, nullptr);

	const XMFLOAT4& color = { 0.0f, 0.0f, 0.0f, 0.0f };
	cmdList->ClearRenderTargetView(mMsaaRtvHandle, (FLOAT*)&color, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE pd3dAllRtvCPUHandles[2] = { mMsaaRtvHandle, mMsaaVelocityMapRtvHandle };

	cmdList->OMSetRenderTargets(2, pd3dAllRtvCPUHandles, FALSE, &depthStencilView);

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

	mpUI.get()->Draw(nFrame);
}

void InGameScene::RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex, bool cubeMapping)
{	
	SetCBV(cmdList, cameraCBIndex);
	mShadowMapRenderer->SetShadowMapSRV(cmdList, 6);

	for (const auto& [layer, pso] : mPipelines)
	{
		/*if (layer != Layer::Terrain && layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, mCurrentCamera->GetWorldFrustum(), true, (bool)mLODSet);
		else if (layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, mCurrentCamera->GetWorldFrustum(), false, (bool)mLODSet);
		else*/
			pso->SetAndDraw(cmdList, (bool)mLODSet, true, cubeMapping);
	}
}

void InGameScene::RenderPipelines(ID3D12GraphicsCommandList* cmdList, Camera* camera, int cameraCBIndex, bool cubeMapping)
{
	SetCBV(cmdList, cameraCBIndex);
	mShadowMapRenderer->SetShadowMapSRV(cmdList, 6);

	for (const auto& [layer, pso] : mPipelines)
	{
		if (layer == Layer::Color)
			continue;

		if (layer != Layer::Terrain && layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, camera->GetWorldFrustum(), true, (bool)mLODSet, true, cubeMapping);
		else if(layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, camera->GetWorldFrustum(), false, (bool)mLODSet, true, cubeMapping);
		else
			pso->SetAndDraw(cmdList, (bool)mLODSet, true, cubeMapping);
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
			missile->SetActive(true);
			mPipelines[Layer::Default]->AppendObject(mMissileObjects[i]);
			missile->SetUpdateFlag(UPDATE_FLAG::NONE);

			flag = true;
			break;
		}
		case UPDATE_FLAG::REMOVE:
		{
			flag = true;
			missile->SetActive(false);
			missile->RemoveObject(*mDynamicsWorld, *mPipelines[Layer::Default]);
			//mMissileObjects[i].reset();
			break;
		}
		case UPDATE_FLAG::NONE:
			continue;
		}
	}
	if (flag) mPipelines[Layer::Default]->ResetPipeline(mDevice.Get());
}

void InGameScene::UpdatePlayerObjects()
{
	bool removed_flag = false;
	for (int i = 0; i < mPlayerObjects.size(); i++)
	{
		auto player = mPlayerObjects[i].get();
		if (player == nullptr) continue;

		switch(player->GetUpdateFlag())
		{
		case UPDATE_FLAG::CREATE:
		{
			player->SetUpdateFlag(UPDATE_FLAG::NONE);
			break;
		}
		case UPDATE_FLAG::REMOVE:
		{
			removed_flag = true;
			mMissileObjects[i]->SetUpdateFlag(UPDATE_FLAG::REMOVE);
			player->RemoveObject(*mDynamicsWorld, *mPipelines[Layer::Color]);
			mPlayerObjects[i].reset();
			break;
		}
		case UPDATE_FLAG::NONE:
			continue;
		}
	}
	if (removed_flag) mPipelines[Layer::Color]->ResetPipeline(mDevice.Get());
}

void InGameScene::LoadWorldMap(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, const std::wstring& path)
{
	std::ifstream in_file{ path };
	std::string info;

	btCompoundShape* compound = new btCompoundShape();

	while (std::getline(in_file, info))
	{
		std::stringstream ss(info);

		std::string objName;
		ss >> objName;

		XMFLOAT3 pos;
		ss >> pos.x >> pos.y >> pos.z;

		XMFLOAT4 quaternion;
		ss >> quaternion.x >> quaternion.y >> quaternion.z >> quaternion.w;

		XMFLOAT3 scale;
		ss >> scale.x >> scale.y >> scale.z;

		auto tmpstr = std::string("Models\\") + objName;

		auto transparentpath = tmpstr;
		transparentpath.replace(tmpstr.find(".obj"), 4, "_Transparent.obj");

		wstring objPath;
		objPath.assign(tmpstr.begin(), tmpstr.end());

		wstring transparentObjPath;
		transparentObjPath.assign(transparentpath.begin(), transparentpath.end());

		
		auto obj = make_shared<StaticObject>();

		if (static_cast<InstancingPipeline*>(mPipelines[Layer::Instancing].get())->mInstancingCount[objName] == 0)
		{
			obj->LoadModel(mDevice.Get(), cmdList, objPath, true);
		}

		btTransform btLocalTransform;
		btLocalTransform.setIdentity();
		btLocalTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
		btLocalTransform.setRotation(btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w));

		auto& meshes = obj->GetMesh();
		for (auto i = meshes.begin(); i < meshes.end(); ++i)
		{
			if (i->get()->GetMeshShape())
			{
				i->get()->GetMeshShape()->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
				compound->addChildShape(btLocalTransform, i->get()->GetMeshShape().get());
			}
		}

		mMeshList[objName] = obj->GetMesh();

		wstring convexObjPath;
		tmpstr.erase(tmpstr.end() - 4, tmpstr.end());
		convexObjPath.assign(tmpstr.begin(), tmpstr.end());

		obj->LoadConvexHullShape(convexObjPath + L"_Convex_Hull.obj", physics);
		obj->SetQuaternion(quaternion);
		obj->SetPosition(pos);
		obj->Scale(scale);
		obj->SetName(objName);

		mPipelines[Layer::Instancing]->AppendObject(obj);
		static_cast<InstancingPipeline*>(mPipelines[Layer::Instancing].get())->mInstancingCount[objName]++;

		if (_access(transparentpath.c_str(), 0) != -1)
		{
			auto transparentObj = make_shared<StaticObject>();
			transparentObj->LoadModel(mDevice.Get(), cmdList, transparentObjPath, true);

			auto& transparentMeshes = transparentObj->GetMesh();
			for (auto i = transparentMeshes.begin(); i < transparentMeshes.end(); ++i)
			{
				if (i->get()->GetMeshShape())
				{
					compound->addChildShape(btLocalTransform, i->get()->GetMeshShape().get());
				}
			}

			transparentObj->SetQuaternion(quaternion);
			transparentObj->SetPosition(pos);
			transparentObj->Scale(scale);
			transparentObj->SetName(objName);

			mPipelines[Layer::Transparent]->AppendObject(transparentObj);
			static_cast<InstancingPipeline*>(mPipelines[Layer::Transparent].get())->mInstancingCount[objName]++;
		}
	}

	btTransform btObjectTransform;
	btObjectTransform.setIdentity();
	btObjectTransform.setOrigin(btVector3(0, 0, 0));

	mTrackRigidBody = physics->CreateRigidBody(0.0f, btObjectTransform, compound);
}
