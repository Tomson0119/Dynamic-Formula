#include "stdafx.h"
#include "gameFramework.h"
#include "camera.h"

#include "InGameUI.h"
#include "LobbyUI.h"
#include "RoomUI.h"
#include "LoginUI.h"

#include "loginScene.h"
#include "lobbyScene.h"
#include "roomScene.h"
#include "inGameScene.h"
#include "NetLib/NetModule.h"

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

	InitScene(SCENE_STAT::LOGIN);
	
	return true;
}

void GameFramework::OnResize()
{
	if (!mScenes.empty())
	{
		mScenes.top().get()->GetUI()->Flush();
		mScenes.top().get()->GetUI()->Reset();
	}
	D3DFramework::OnResize();
	if (!mScenes.empty()) 
	{ 
		auto ui = mScenes.top().get()->GetUI();
		//ui->Reset();
		mScenes.top()->OnResize(GetAspect()); 
		ui->OnResize(mSwapChainBuffers->GetAddressOf(), mD3dDevice, mCommandQueue.Get(), mSwapChainBufferCount, gFrameWidth, gFrameHeight);
		mScenes.top()->ProcessAfterResize();
	}
}

void GameFramework::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
	mScenes.top()->OnProcessMouseDown(buttonState, x, y);
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
			//if (mScenes.size() > 0);
				//mScenes.pop();

			OutputDebugStringA("Escape key inserted\n");
			if (mScenes.empty()) {
				PostQuitMessage(0);
				return;
			}
			break;

		case VK_F9:
			mScenes.top()->GetUI()->Flush();
			mScenes.top()->GetUI()->Reset();
			D3DFramework::ChangeFullScreenState();
			mScenes.top().get()->GetUI()->OnResize(mSwapChainBuffers->GetAddressOf(), mD3dDevice, mCommandQueue.Get(), mSwapChainBufferCount, gFrameWidth, gFrameHeight);

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

	if (!mScenes.empty())
	{
		mScenes.top()->GetUI()->Flush();
		mScenes.top()->GetUI()->Reset();
	}

	switch (state)
	{
	case SCENE_STAT::LOGIN:
		mScenes.push(std::make_unique<LoginScene>(mHwnd, mNetwork.get()));
		break;

	case SCENE_STAT::LOBBY:
		mScenes.push(std::make_unique<LobbyScene>(mHwnd, mNetwork.get()));
		break;

	case SCENE_STAT::ROOM:
		mScenes.push(std::make_unique<RoomScene>(mHwnd, mNetwork.get()));
		break;

	case SCENE_STAT::IN_GAME:
		mScenes.push(std::make_unique<InGameScene>(mHwnd, mNetwork.get(), mMsaa4xEnable, mMsaa4xQualityLevels));
		break;

	default:
		OutputDebugStringW(L"Wrong scene!!.\n");
		break;
	}

	//mScenes.top()->BuildObjects(mD3dDevice, mCommandList.Get(), GetAspect(), mBulletPhysics);
	mScenes.top()->BuildObjects(mD3dDevice.Get(), mCommandList.Get(), mCommandQueue.Get(), mSwapChainBufferCount, mSwapChainBuffers->GetAddressOf(), static_cast<float>(gFrameWidth), static_cast<float>(gFrameHeight), GetAspect(), mBulletPhysics);

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	WaitUntilGPUComplete();
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
		mTimer.Reset();
		break;
	}
	case SCENE_CHANGE_FLAG::POP:
	{
		mScenes.top()->SetSceneChangeFlag(SCENE_CHANGE_FLAG::NONE);
		mScenes.pop();
		mScenes.top()->Reset();
		mScenes.top()->GetUI()->SetStatePop(mSwapChainBufferCount, mD3dDevice, mCommandQueue.Get(), mSwapChainBuffers->GetAddressOf(), gFrameWidth, gFrameHeight);
		mNetwork->SetInterface(mScenes.top().get());
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
	D3DFramework::UpdateFrameStates();
	mScenes.top()->Update(mCommandList.Get(), mTimer, mBulletPhysics);
}

void GameFramework::Draw()
{
	CheckAndChangeScene();

	// ��ɾ� �Ҵ��ڸ� ���� �ʱ�ȭ���ش�.
	ThrowIfFailed(mCommandAllocator->Reset());

	// Command List�� Pipeline State�� ���´�. 
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	if(mScenes.top()->GetRootSignature())
		mCommandList->SetGraphicsRootSignature(mScenes.top()->GetRootSignature());

	Update();

	mScenes.top()->PreRender(mCommandList.Get(), mTimer.ElapsedTime());

	mCommandList->RSSetViewports(1, &mViewPort);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// ȭ�� ������ ���¸� Render Target ���·� �����Ѵ�.
	mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// ȭ�� ���ۿ� ���� ���ٽ� ���۸� �ʱ�ȭ�Ѵ�.
	const XMFLOAT4& color = mScenes.top()->GetFrameColor();

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (FLOAT*)&color, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// �������� ���۸� ��ü������ �����Ѵ�.

	mScenes.top()->Draw(mCommandList.Get(), CurrentBackBufferView(), DepthStencilView(), CurrentBackBuffer(), mDepthStencilBuffer.Get(), mCurrBackBufferIndex);

	// ȭ�� ������ ���¸� �ٽ� PRESENT ���·� �����Ѵ�.
	/*mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));*/

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	
	auto ui = mScenes.top()->GetUI();
	if(ui) ui->Flush();

	// Ŀ�ǵ� ����Ʈ�� ��ɾ���� �� �����ϱ���� ��ٸ���.
	WaitUntilGPUComplete();

	ThrowIfFailed(mD3dDevice->GetDeviceRemovedReason());
	ThrowIfFailed(mSwapChain->Present(0, 0));  // ȭ����۸� Swap�Ѵ�.	
	
	// ���� �ĸ���� ��ġ�� �̵��� �� �ٽ� ��ٸ���.
	mCurrBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	
	WaitUntilGPUComplete();
}