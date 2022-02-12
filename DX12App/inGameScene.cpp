#include "stdafx.h"
#include "inGameScene.h"
#include "shadowMapRenderer.h"
#include "NetLib/NetModule.h"

using namespace std;

InGameScene::InGameScene(NetModule* netPtr)
	: Scene(SCENE_STAT::IN_GAME, (XMFLOAT4)Colors::White, netPtr)
{
	OutputDebugStringW(L"In Game Scene Entered.\n");
	
	mKeyMap[VK_LEFT] = false;
	mKeyMap[VK_RIGHT] = false;
	mKeyMap[VK_UP] = false;
	mKeyMap[VK_DOWN] = false;
	mKeyMap[VK_LSHIFT] = false;
	mKeyMap['Z'] = false;
	mKeyMap['X'] = false;
}

InGameScene::~InGameScene()
{
}

void InGameScene::OnResize(float aspect)
{
	if(mMainCamera)
		mMainCamera->SetLens(aspect);
}

void InGameScene::BuildObjects(ComPtr<ID3D12Device> device, ID3D12GraphicsCommandList* cmdList, float aspect, std::shared_ptr<BulletWrapper> physics)
{
	mDevice = device;

	mMainCamera = make_unique<Camera>();
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 4000.0f);
	mMainCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3( 0.0f,0.0f,0.0f ), XMFLOAT3( 0.0f,1.0f,0.0f ));
	mMainCamera->SetPosition(0.0f, 0.0f, 0.0f);
	mMainCamera->Move(mMainCamera->GetLook(), -mCameraRadius);

	mMainLight.Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mMainLight.Lights[0].SetInfo(
		XMFLOAT3(0.6f, 0.6f, 0.6f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, -1.0f),
		3000.0f, DIRECTIONAL_LIGHT);
	mMainLight.Lights[1].SetInfo(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, -1.0f),
		3000.0f, DIRECTIONAL_LIGHT);
	mMainLight.Lights[2].SetInfo(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, 1.0f),
		3000.0f, DIRECTIONAL_LIGHT);

	BuildRootSignature();
	BuildComputeRootSignature();
	BuildShadersAndPSOs(cmdList);
	BuildGameObjects(cmdList, physics);
	BuildConstantBuffers();
	BuildDescriptorHeap();
}

void InGameScene::BuildRootSignature()
{
	D3D12_DESCRIPTOR_RANGE descRanges[4];
	descRanges[0] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4);
	descRanges[1] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
	descRanges[2] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1);
	descRanges[3] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);

	D3D12_ROOT_PARAMETER parameters[9];
	parameters[0] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 0, D3D12_SHADER_VISIBILITY_ALL);    // CameraCB
	parameters[1] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);    // LightCB
	parameters[2] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 2, D3D12_SHADER_VISIBILITY_ALL);    // GameInfoCB
	parameters[3] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 3, D3D12_SHADER_VISIBILITY_ALL);	 // MaterialCB
	parameters[4] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);			     // Object,  CBV
	parameters[5] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_ALL);				 // Texture, SRV
	parameters[6] = Extension::DescriptorTable(1, &descRanges[2], D3D12_SHADER_VISIBILITY_ALL);				 // ShadowMap
	parameters[7] = Extension::DescriptorTable(1, &descRanges[3], D3D12_SHADER_VISIBILITY_ALL);				 // CubeMap
	parameters[8] = Extension::Constants(3, 5, D3D12_SHADER_VISIBILITY_ALL);                                 // Shadow ViewProjection

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

	D3D12_ROOT_PARAMETER parameters[2];
	parameters[0] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);    // Inputs
	parameters[1] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_ALL);    // Output																   

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
	auto colorShader = make_unique<DefaultShader>(L"Shaders\\color.hlsl");
	auto terrainShader = make_unique<TerrainShader>(L"Shaders\\terrain.hlsl");
	auto motionBlurShader = make_unique<ComputeShader>(L"Shaders\\motionBlur.hlsl");

	mPipelines[Layer::SkyBox] = make_unique<SkyboxPipeline>(mDevice.Get(), cmdList);
	mPipelines[Layer::SkyBox]->BuildPipeline(mDevice.Get(), mRootSignature.Get());

	mPipelines[Layer::Default] = make_unique<Pipeline>();
	mPipelines[Layer::Default]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), defaultShader.get());

	mPipelines[Layer::Terrain] = make_unique<Pipeline>();
	mPipelines[Layer::Terrain]->SetWiredFrame(true);
	mPipelines[Layer::Terrain]->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
	mPipelines[Layer::Terrain]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), terrainShader.get());

	mPipelines[Layer::Color] = make_unique<Pipeline>();
	mPipelines[Layer::Color]->BuildPipeline(mDevice.Get(), mRootSignature.Get(), colorShader.get());

	mPostProcessingPipelines[Layer::MotionBlur] = make_unique<ComputePipeline>(mDevice.Get());
	mPostProcessingPipelines[Layer::MotionBlur]->BuildPipeline(mDevice.Get(), mComputeRootSignature.Get(), motionBlurShader.get());

	mShadowMapRenderer = make_unique<ShadowMapRenderer>(mDevice.Get(), 2048, 2048, 3, mMainCamera.get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Default, mPipelines[Layer::Default].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Color, mPipelines[Layer::Color].get());
	mShadowMapRenderer->AppendTargetPipeline(Layer::Terrain, mPipelines[Layer::Terrain].get());
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
	mShadowMapRenderer->BuildDescriptorHeap(mDevice.Get(), 3, 4, 5);
	for (const auto& [_, pso] : mPipelines)
		pso->BuildDescriptorHeap(mDevice.Get(), 3, 4, 5);
}

void InGameScene::CreateVelocityMapViews()
{
	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R32G32B32A32_FLOAT, {0.0f,0.0f,0.0f,0.0f} };

	mVelocityMap = CreateTexture2DResource(
		mDevice.Get(), gFrameWidth, gFrameHeight, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mVelocityMapRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mDevice->CreateRenderTargetView(mVelocityMap.Get(), nullptr, rtvHandle);
	mVelocityMapRtvHandle = rtvHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = mVelocityMapSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mDevice->CreateShaderResourceView(mVelocityMap.Get(), &srvDesc, srvHandle);
	mVelocityMapSrvHandle = srvHandle;
}

void InGameScene::CreateVelocityMapDescriptorHeaps()
{
	ThrowIfFailed(mDevice->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			1,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
		IID_PPV_ARGS(&mVelocityMapRtvDescriptorHeap)));

	ThrowIfFailed(mDevice->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			1,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mVelocityMapSrvDescriptorHeap)));
}

void InGameScene::BuildGameObjects(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<BulletWrapper>& physics)
{
	mDynamicsWorld = physics->GetDynamicsWorld();

	mMeshList[MeshType::Missile].push_back(std::make_shared<BoxMesh>(mDevice.Get(), cmdList, 5, 5, 5));
	mMeshList[MeshType::Grid].push_back(make_shared<GridMesh>(mDevice.Get(), cmdList, 50.0f, 50.0f, 10.0f, 10.0f));

	auto grid = make_shared<GameObject>();
	grid->SetMeshes(mMeshList[MeshType::Grid]);
	grid->LoadTexture(mDevice.Get(), cmdList, L"Resources\\tile.dds");
	grid->Rotate(90.0f, 0.0f, 0.0f);
	mPipelines[Layer::Default]->AppendObject(grid);

	auto terrain = make_shared<TerrainObject>(1024, 1024, XMFLOAT3(1.0f, 1.0f, 1.0f));
	//terrain->BuildHeightMap(L"Resources\\heightmap.raw");
	terrain->BuildHeightMap(L"Resources\\PlaneMap.raw");
	terrain->BuildTerrainMesh(mDevice.Get(), cmdList, physics, 89, 89);
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\terrainTexture.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\rocky.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\road.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\heightmap.dds");
	terrain->LoadTexture(mDevice.Get(), cmdList, L"Resources\\normalmap.dds");
	mPipelines[Layer::Terrain]->AppendObject(terrain);

#ifdef STANDALONE
	BuildCarObjects({ 500.0f, 300.0f, 500.0f }, 4, true, cmdList, physics, 0);
#else
	const auto& players = mNetPtr->GetPlayersInfo();
	for (int i = 0; const PlayerInfo& info : players)
	{
		if (info.Empty == false)
		{
			bool isPlayer = (i == mNetPtr->GetPlayerIndex()) ? true : false;
			BuildCarObjects(info.StartPosition, info.Color, isPlayer, cmdList, physics, i);
		}
		i++;
	}
#endif
	float aspect = mMainCamera->GetAspect();
	mMainCamera.reset(mPlayer->ChangeCameraMode((int)CameraMode::THIRD_PERSON_CAMERA));
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 2000.0f);
}

void InGameScene::BuildCarObjects(
	const XMFLOAT3& position,
	char color,
	bool isPlayer,
	ID3D12GraphicsCommandList* cmdList, 
	std::shared_ptr<BulletWrapper>& physics,
	UINT netID)
{
	auto carObj = make_shared<PhysicsPlayer>(netID);
	carObj->SetPosition(position);

	if (mMeshList[MeshType::Car].empty())
		mMeshList[MeshType::Car] = carObj->LoadModel(mDevice.Get(), cmdList, L"Models\\Car_Body.obj");
	else
		carObj->SetMeshes(mMeshList[MeshType::Car]);

	carObj->SetDiffuse("Car_Texture", mColorMap[(int)color]);
	for (int i = 0; i < 4; ++i)
	{
		auto wheelObj = make_shared<WheelObject>();

		if (i % 2 == 0)
		{
			if (mMeshList[MeshType::Wheel_L].empty())
				mMeshList[MeshType::Wheel_L] = wheelObj->LoadModel(mDevice.Get(), cmdList, L"Models\\Car_Wheel_L.obj");
			else
				wheelObj->SetMeshes(mMeshList[MeshType::Wheel_L]);
		}
		else
		{
			if (mMeshList[MeshType::Wheel_R].empty())
				mMeshList[MeshType::Wheel_R] = wheelObj->LoadModel(mDevice.Get(), cmdList, L"Models\\Car_Wheel_R.obj");
			else
				wheelObj->SetMeshes(mMeshList[MeshType::Wheel_R]);
		}

		carObj->SetWheel(wheelObj.get(), i);
		mPipelines[Layer::Color]->AppendObject(wheelObj);
	}
	carObj->BuildRigidBody(physics);
	carObj->BuildDsvRtvView(mDevice.Get());
	if (isPlayer) mPlayer = carObj.get();
	mPipelines[Layer::Color]->AppendObject(carObj);
	mPlayerObjects.push_back(carObj);
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
	case SC::REMOVE_PLAYER:
	{
		SC::packet_remove_player* pck = reinterpret_cast<SC::packet_remove_player*>(packet);
		mNetPtr->RemovePlayer(pck);

		bool flag = false;
		for (auto i = mPlayerObjects.begin(); i < mPlayerObjects.end();)
		{
			if (i->get()->GetNetID() == pck->player_idx)
			{
				flag = true;
				mDynamicsWorld->removeRigidBody(i->get()->GetRigidBody());
				auto& colorObjects = mPipelines[Layer::Color]->GetRenderObjects();
				for (int j = 0; j < colorObjects.size(); ++j)
				{
					if (*i == colorObjects[j])
					{
						mPipelines[Layer::Color]->DeleteObject(j);
					}
				}

				i = mPlayerObjects.erase(i);
			}
			else
				++i;
		}
		if (flag) mPipelines[Layer::Color]->ResetPipeline(mDevice.Get());
		break;
	}
	}
	return true;
}

void InGameScene::OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y)
{
	if ((buttonState) && !GetCapture())
	{
		SetCapture(hwnd);
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

		mMainCamera->Pitch(0.25f * dy);
		mMainCamera->RotateY(0.25f * dx);
	}
}

void InGameScene::OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

void InGameScene::OnPreciseKeyInput(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<BulletWrapper> physics, float elapsed)
{
#ifdef STANDALONE
	if (mMissileInterval < 0.0f)
	{
		if (GetAsyncKeyState('X') & 0x8000)
		{
			mMissileInterval = 1.0f;
			AppendMissileObject(cmdList, physics);
		}
	}
	else
	{
		mMissileInterval -= elapsed;
	}
	
	if(mPlayer) mPlayer->OnPreciseKeyInput(elapsed);
#else
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

void InGameScene::Update(ID3D12GraphicsCommandList* cmdList, const GameTimer& timer, std::shared_ptr<BulletWrapper> physics)
{
	float elapsed = timer.ElapsedTime();

	OnPreciseKeyInput(cmdList, physics, elapsed);

	UpdateLight(elapsed);
	mMainCamera->Update(elapsed);

	mShadowMapRenderer->UpdateSplitFrustum(mMainCamera.get());
	mShadowMapRenderer->UpdateDepthCamera(cmdList, mMainLight);

	for (const auto& [_, pso] : mPipelines)
		pso->Update(elapsed, mMainCamera.get());

	UpdateMissileObject();
	
	UpdateConstants(timer);
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
	UpdateCameraConstant(0, mMainCamera.get());
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

void InGameScene::SetCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex)
{
	cmdList->SetGraphicsRootConstantBufferView(0, mCameraCB->GetGPUVirtualAddress(cameraCBIndex));
	cmdList->SetGraphicsRootConstantBufferView(1, mLightCB->GetGPUVirtualAddress(0));
	cmdList->SetGraphicsRootConstantBufferView(2, mGameInfoCB->GetGPUVirtualAddress(0));
}

void InGameScene::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer)
{
	const XMFLOAT4& velocity = { 0.0f, 0.0f, 0.0f, 0.0f };
	cmdList->ClearRenderTargetView(mVelocityMapRtvHandle, (FLOAT*)&velocity, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE pd3dAllRtvCPUHandles[2] = { backBufferview, mVelocityMapRtvHandle };

	cmdList->OMSetRenderTargets(2, pd3dAllRtvCPUHandles, FALSE, &depthStencilView);

	cmdList->SetGraphicsRootSignature(mRootSignature.Get());
	RenderPipelines(cmdList, 0);

	mPostProcessingPipelines[Layer::MotionBlur]->SetInput(cmdList, mVelocityMap.Get(), 0);
	mPostProcessingPipelines[Layer::MotionBlur]->SetInput(cmdList, backBuffer, 1);

	mPostProcessingPipelines[Layer::MotionBlur]->Dispatch(cmdList);

	mPostProcessingPipelines[Layer::MotionBlur]->CopyMapToRT(cmdList, backBuffer);
}

void InGameScene::RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex)
{	
	SetCBV(cmdList, cameraCBIndex);
	mShadowMapRenderer->SetShadowMapSRV(cmdList, 6);

	for (const auto& [layer, pso] : mPipelines)
	{
		if (layer != Layer::Terrain && layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, mMainCamera->GetWorldFrustum(), true, (bool)mLODSet);
		else if (layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, mMainCamera->GetWorldFrustum(), false, (bool)mLODSet);
		else
			pso->SetAndDraw(cmdList, (bool)mLODSet);
	}
}

void InGameScene::RenderPipelines(ID3D12GraphicsCommandList* cmdList, Camera* camera ,int cameraCBIndex)
{
	SetCBV(cmdList, cameraCBIndex);
	mShadowMapRenderer->SetShadowMapSRV(cmdList, 6);

	for (const auto& [layer, pso] : mPipelines)
	{
		if (layer == Layer::Color)
			continue;

		if (layer != Layer::Terrain && layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, camera->GetWorldFrustum(), true, (bool)mLODSet);
		else if(layer != Layer::SkyBox)
			pso->SetAndDraw(cmdList, camera->GetWorldFrustum(), false, (bool)mLODSet);
		else
			pso->SetAndDraw(cmdList, (bool)mLODSet);
	}
}

void InGameScene::AppendMissileObject(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<BulletWrapper> physics)
{
	std::shared_ptr<MissileObject> missile = std::make_shared<MissileObject>();
	missile->SetMesh(mMeshList[MeshType::Missile][0], mPlayer->GetVehicle()->getForwardVector(), mPlayer->GetPosition(), physics);
	missile->LoadTexture(mDevice.Get(), cmdList, L"Resources\\tile.dds");

	mMissileObjects.push_back(missile);
	mPipelines[Layer::Default]->AppendObject(missile);
	mPipelines[Layer::Default]->ResetPipeline(mDevice.Get());
}

void InGameScene::UpdateMissileObject()
{
	bool flag = false;
	for (auto i = mMissileObjects.begin(); i < mMissileObjects.end();)
	{
		if (i->get()->GetDuration() < 0.0f)
		{
			flag = true;
			mDynamicsWorld->removeRigidBody(i->get()->GetRigidBody());
			auto& defaultObjects = mPipelines[Layer::Default]->GetRenderObjects();
			for (int j = 0; j < defaultObjects.size(); ++j)
			{
				if (*i == defaultObjects[j])
				{
					mPipelines[Layer::Default]->DeleteObject(j);
				}
			}

			i = mMissileObjects.erase(i);
		}
		else
			++i;
	}
	if (flag) mPipelines[Layer::Default]->ResetPipeline(mDevice.Get());
}


