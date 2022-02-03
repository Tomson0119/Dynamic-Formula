#include "stdafx.h"
#include "gameFramework.h"
#include "camera.h"
#include "UI.h"

#include "loginScene.h"
#include "lobbyScene.h"
#include "roomScene.h"
#include "inGameScene.h"

GameFramework::GameFramework()
	: D3DFramework()
{
}

GameFramework::~GameFramework()
{
	size_t size = mScenes.size();
	for (int i = 0; i < size; ++i) mScenes.pop();
}

bool GameFramework::InitFramework()
{
	if (!D3DFramework::InitFramework())
		return false;
	////UI Build
	//if (!mpUI)
	//{
	//	mpUI = new UI(mSwapChainBufferCount, mD3dDevice.Get(), mCommandQueue.Get());
	//	mpUI->Resize(mSwapChainBuffers->GetAddressOf(), gFrameWidth, gFrameHeight);
	//}
	InitScene(SCENE_STAT::LOGIN);
	return true;
}

void GameFramework::OnResize()
{
	D3DFramework::OnResize();
	if (!mScenes.empty()) mScenes.top()->OnResize(GetAspect());
}

void GameFramework::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
	mScenes.top()->OnProcessMouseDown(m_hwnd, buttonState, x, y);
}

void GameFramework::OnProcessMouseUp(WPARAM buttonState, int x, int y)
{
	mScenes.top()->OnProcessMouseUp(buttonState, x, y);
}

void GameFramework::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
	mScenes.top()->OnProcessMouseMove(buttonState, x, y);
}

void GameFramework::OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (mScenes.size() > 0);
				//mScenes.pop();

			if (mScenes.empty()) {
				PostQuitMessage(0);
				return;
			}
			break;

		case VK_F9:
			D3DFramework::ChangeFullScreenState();
			break;
		}
		break;
	}
	mScenes.top()->OnProcessKeyInput(uMsg, wParam, lParam);
}

void GameFramework::InitScene(SCENE_STAT state)
{
	ThrowIfFailed(mCommandAllocator->Reset());
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	switch (state)
	{
	case SCENE_STAT::LOGIN:
		mScenes.push(std::make_unique<LoginScene>(mNetwork.get()));
		break;

	case SCENE_STAT::LOBBY:
		mScenes.push(std::make_unique<LobbyScene>(mNetwork.get()));
		break;

	case SCENE_STAT::ROOM:
		mScenes.push(std::make_unique<RoomScene>(mNetwork.get()));
		break;

	case SCENE_STAT::IN_GAME:
		mScenes.push(std::make_unique<InGameScene>(mNetwork.get()));
		break;

	default:
		OutputDebugStringW(L"Wrong scene!!.\n");
		break;
	}

	mScenes.top()->BuildObjects(mD3dDevice.Get(), mCommandList.Get(), GetAspect(), mBtDynamicsWorld);

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	WaitUntilGPUComplete();
}

void GameFramework::OnPreciseKeyInput()
{
}

void GameFramework::CheckAndChangeScene()
{
	switch (mScenes.top()->GetSceneChangeFlag())
	{
	case SCENE_CHANGE_FLAG::PUSH:
	{
		mScenes.top()->SetSceneChangeFlag(SCENE_CHANGE_FLAG::NONE);
		char nextScene = static_cast<char>(mScenes.top()->GetSceneState()) + 1;
		InitScene(static_cast<SCENE_STAT>(nextScene));
		break;
	}
	case SCENE_CHANGE_FLAG::POP:
	{
		mScenes.top()->SetSceneChangeFlag(SCENE_CHANGE_FLAG::NONE);
		mScenes.pop();
		break;
	}
	case SCENE_CHANGE_FLAG::LOGOUT:
	{
		mScenes.top()->SetSceneChangeFlag(SCENE_CHANGE_FLAG::NONE);
		while (mScenes.size() > 1) mScenes.pop();
		break;
	}
	default:
		break;
	}
}

void GameFramework::Update()
{
	if(mBtDynamicsWorld) 
		mBtDynamicsWorld->stepSimulation(mTimer.ElapsedTime());

	D3DFramework::UpdateFrameStates();
	
	OnPreciseKeyInput();

	//UI Update
	//UpdateUI();

	//mCamera->Update(mTimer.ElapsedTime());
	mScenes.top()->Update(mD3dDevice.Get(), mCommandList.Get(), mTimer, mBtDynamicsWorld);
}

void GameFramework::Draw()
{
	CheckAndChangeScene();

	// 명령어 할당자를 먼저 초기화해준다.
	ThrowIfFailed(mCommandAllocator->Reset());

	// Command List를 Pipeline State로 묶는다. 
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	mCommandList->SetGraphicsRootSignature(mScenes.top()->GetRootSignature());

	Update();

	mScenes.top()->PreRender(mCommandList.Get(), mTimer.ElapsedTime());

	mCommandList->RSSetViewports(1, &mViewPort);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 화면 버퍼의 상태를 Render Target 상태로 전이한다.
	mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 화면 버퍼와 깊이 스텐실 버퍼를 초기화한다.
	const XMFLOAT4& color = mScenes.top()->GetFrameColor();

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (FLOAT*)&color, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 렌더링할 버퍼를 구체적으로 설정한다.

	mScenes.top()->Draw(mCommandList.Get(), CurrentBackBufferView(), DepthStencilView(), CurrentBackBuffer());

	// 화면 버퍼의 상태를 다시 PRESENT 상태로 전이한다.
	mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	//for (auto ui : mpUI)
	//mpUI->Draw(mCurrBackBufferIndex);
	// 커맨드 리스트의 명령어들을 다 실행하기까지 기다린다.
	WaitUntilGPUComplete();

	ThrowIfFailed(mD3dDevice->GetDeviceRemovedReason());
	ThrowIfFailed(mSwapChain->Present(0, 0));  // 화면버퍼를 Swap한다.	

	
	// 다음 후면버퍼 위치로 이동한 후 다시 기다린다.
	mCurrBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	
	WaitUntilGPUComplete();
}

