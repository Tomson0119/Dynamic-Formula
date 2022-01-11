#include "stdafx.h"
#include "gameScene.h"
#include "dynamicCubeRenderer.h"
#include "shadowMapRenderer.h"

using namespace std;

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
}

void GameScene::OnResize(float aspect)
{
	if(mMainCamera)
		mMainCamera->SetLens(aspect);
}

void GameScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float aspect)
{
	mMainCamera = make_unique<Camera>();
	mMainCamera->SetLens(0.25f * Math::PI, aspect, 1.0f, 5000.0f);
	mMainCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -10.0f), XMFLOAT3( 0.0f,0.0f,0.0f ), XMFLOAT3( 0.0f,1.0f,0.0f ));
	mMainCamera->SetPosition(0.0f, 0.0f, 0.0f);
	mMainCamera->Move(mMainCamera->GetLook(), -mCameraRadius);

	mMainLight.Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mMainLight.Lights[0].SetInfo(
		XMFLOAT3(0.6f, 0.6f, 0.6f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.75f, -1.0f),
		3000.0f, DIRECTIONAL_LIGHT);
	mMainLight.Lights[1].SetInfo(
		XMFLOAT3(0.3f, 0.3f, 0.3f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, -1.0f),
		3000.0f, DIRECTIONAL_LIGHT);
	mMainLight.Lights[2].SetInfo(
		XMFLOAT3(0.15f, 0.15f, 0.15f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.75f, 1.0f),
		3000.0f, DIRECTIONAL_LIGHT);

	BuildRootSignature(device);
	BuildShadersAndPSOs(device, cmdList);
	BuildGameObjects(device, cmdList);
	BuildConstantBuffers(device);
	BuildDescriptorHeap(device);
}

void GameScene::BuildRootSignature(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_RANGE descRanges[3];
	descRanges[0] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4);
	descRanges[1] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
	descRanges[2] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

	D3D12_ROOT_PARAMETER parameters[7];
	parameters[0] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 0, D3D12_SHADER_VISIBILITY_ALL);    // CameraCB
	parameters[1] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);    // LightCB
	parameters[2] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 2, D3D12_SHADER_VISIBILITY_ALL);    // GameInfoCB
	parameters[3] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 3, D3D12_SHADER_VISIBILITY_ALL);	 // MaterialCB
	parameters[4] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);			     // Object,  CBV
	parameters[5] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_ALL);				 // Texture, SRV
	parameters[6] = Extension::DescriptorTable(1, &descRanges[2], D3D12_SHADER_VISIBILITY_ALL);				 // ShadowMap 																	   
    
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

	ThrowIfFailed(device->CreateRootSignature(
		0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void GameScene::BuildShadersAndPSOs(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	auto defaultShader = make_unique<DefaultShader>(L"Shaders\\default.hlsl");
	//auto colorShader = make_unique<DefaultShader>(L"Shaders\\color.hlsl");

	mPipelines[Layer::SkyBox] = make_unique<SkyboxPipeline>(device, cmdList);
	mPipelines[Layer::SkyBox]->BuildPipeline(device, mRootSignature.Get());

	mPipelines[Layer::Default] = make_unique<Pipeline>();
	//mPipelines[Layer::Default]->SetCullClockwise();
	mPipelines[Layer::Default]->BuildPipeline(device, mRootSignature.Get(), defaultShader.get());

	/*mPipelines[Layer::Color] = make_unique<Pipeline>();
	mPipelines[Layer::Color]->BuildPipeline(device, mRootSignature.Get(), colorShader.get());*/
}

void GameScene::BuildConstantBuffers(ID3D12Device* device)
{
	mLightCB = std::make_unique<ConstantBuffer<LightConstants>>(device, 2);
	mCameraCB = std::make_unique<ConstantBuffer<CameraConstants>>(device, 1 + 2);
	mGameInfoCB = std::make_unique<ConstantBuffer<GameInfoConstants>>(device, 1);

	for (const auto& [_, pso] : mPipelines)
		pso->BuildConstantBuffer(device);
}

void GameScene::BuildDescriptorHeap(ID3D12Device* device)
{
	for (const auto& [_, pso] : mPipelines)
		pso->BuildDescriptorHeap(device, 3, 4, 5);
}

void GameScene::BuildGameObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	auto box = make_shared<GameObject>();
	box->LoadModel(device, cmdList, L"Models\\road_sign2.obj");
	box->Move(0.0f, 10.0f, 0.0f);
	mPipelines[Layer::Default]->AppendObject(box);

	auto gridMesh = make_shared<GridMesh>(device, cmdList, 50.0f, 50.0f, 10.0f, 10.0f);
	auto grid = make_shared<GameObject>();
	grid->SetMesh(gridMesh);
	grid->LoadTexture(device, cmdList, L"Resources\\tile.dds");
	grid->Rotate(90.0f, 0.0f, 0.0f);
	mPipelines[Layer::Default]->AppendObject(grid);
}

void GameScene::PreRender(ID3D12GraphicsCommandList* cmdList)
{
}

void GameScene::OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y)
{
	if ((buttonState) && !GetCapture())
	{
		SetCapture(hwnd);
		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}
}

void GameScene::OnProcessMouseUp(WPARAM buttonState, int x, int y)
{
	ReleaseCapture();
}

void GameScene::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
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

void GameScene::OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

void GameScene::OnPreciseKeyInput(float elapsed)
{
	float move_speed = 10.0f;
	if (GetAsyncKeyState('A') & 0x8000)
		mMainCamera->Move(-move_speed * elapsed,0.0f,0.0f);
	if (GetAsyncKeyState('W') & 0x8000)
		mMainCamera->Move(0.0f, 0.0f, +move_speed * elapsed);
	if (GetAsyncKeyState('S') & 0x8000)
		mMainCamera->Move(0.0f, 0.0f, -move_speed * elapsed);
	if (GetAsyncKeyState('D') & 0x8000)
		mMainCamera->Move(+move_speed * elapsed, 0.0f, 0.0f);
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		mMainCamera->Move(0.0f, +move_speed * elapsed, 0.0f);
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
		mMainCamera->Move(0.0f, -move_speed * elapsed, 0.0f);
}

void GameScene::Update(ID3D12Device* device, const GameTimer& timer)
{
	float elapsed = timer.ElapsedTime();

	OnPreciseKeyInput(elapsed);

	UpdateLight(elapsed);
	mMainCamera->Update(elapsed);

	for (const auto& [_, pso] : mPipelines)
		pso->Update(elapsed, mMainCamera.get());

	UpdateConstants(timer);
}

void GameScene::UpdateLight(float elapsed)
{
	XMMATRIX R = XMMatrixRotationY(0.1f * elapsed);
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		// rotate each direction..
		mMainLight.Lights[i].Direction = Vector3::TransformNormal(mMainLight.Lights[i].Direction,R);
		mMainLight.Lights[i].Position = Vector3::Multiply(2.0f, mMainLight.Lights[i].Direction);
	}
}

void GameScene::UpdateLightConstants()
{
	mLightCB->CopyData(0, mMainLight);
}

void GameScene::UpdateCameraConstant(int idx, Camera* camera)
{
	// 카메라로부터 상수를 받는다.
	mCameraCB->CopyData(idx, camera->GetConstants());
}

void GameScene::UpdateConstants(const GameTimer& timer)
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
	gameInfo.KeyInput = mLODSet;
	gameInfo.CurrentTime = timer.CurrentTime();
	gameInfo.ElapsedTime = timer.ElapsedTime();

	mGameInfoCB->CopyData(0, gameInfo);
	
	for (const auto& [_, pso] : mPipelines)
		pso->UpdateConstants();
}

void GameScene::SetCBV(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex)
{
	cmdList->SetGraphicsRootConstantBufferView(0, mCameraCB->GetGPUVirtualAddress(cameraCBIndex));
	cmdList->SetGraphicsRootConstantBufferView(1, mLightCB->GetGPUVirtualAddress(0));
	cmdList->SetGraphicsRootConstantBufferView(2, mGameInfoCB->GetGPUVirtualAddress(0));
}

void GameScene::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer)
{
	cmdList->SetGraphicsRootSignature(mRootSignature.Get());
	RenderPipelines(cmdList, 0);
}

void GameScene::RenderPipelines(ID3D12GraphicsCommandList* cmdList, int cameraCBIndex)
{	
	SetCBV(cmdList, cameraCBIndex);

	for (const auto& [layer, pso] : mPipelines) {
		pso->SetAndDraw(cmdList, (bool)mLODSet);	
	}
}
