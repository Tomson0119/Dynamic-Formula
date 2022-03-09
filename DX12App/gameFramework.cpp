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
	D3DFramework::OnResize();
	if (!mScenes.empty()) 
	{ 
		mScenes.top()->OnResize(GetAspect());

		auto ui = mScenes.top()->GetUI();
		//if(ui) ui->OnResize(mSwapChainBuffers->GetAddressOf(), mD3dDevice, mCommandQueue.Get(), mSwapChainBufferCount, gFrameWidth, gFrameHeight);
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
			if (mScenes.size() > 0);
				//mScenes.pop();

			if (mScenes.empty()) {
				PostQuitMessage(0);
				return;
			}
			break;

		case VK_F9:
			auto ui = mScenes.top()->GetUI();
			//if (ui) ui->Reset();
		
			D3DFramework::ChangeFullScreenState();

			//if(ui) ui->OnResize(mSwapChainBuffers->GetAddressOf(), mD3dDevice, mCommandQueue.Get(), mSwapChainBufferCount, gFrameWidth, gFrameHeight);

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
		mScenes.push(std::make_unique<LoginScene>(m_hwnd, mNetwork.get()));
		break;

	case SCENE_STAT::LOBBY:
		mScenes.push(std::make_unique<LobbyScene>(m_hwnd, mNetwork.get()));
		break;

	case SCENE_STAT::ROOM:
		mScenes.push(std::make_unique<RoomScene>(m_hwnd, mNetwork.get()));
		break;

	case SCENE_STAT::IN_GAME:
		mScenes.push(std::make_unique<InGameScene>(m_hwnd, mNetwork.get()));
		break;

	default:
		OutputDebugStringW(L"Wrong scene!!.\n");
		break;
	}

	//mScenes.top()->BuildObjects(mD3dDevice, mCommandList.Get(), GetAspect(), mBulletPhysics);
	mScenes.top()->BuildObjects(mD3dDevice, mCommandList.Get(), mCommandQueue.Get(), mSwapChainBufferCount, mSwapChainBuffers->GetAddressOf(), gFrameWidth, gFrameHeight, GetAspect(), mBulletPhysics);

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
	D3DFramework::UpdateFrameStates();
	
	OnPreciseKeyInput();

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

	mScenes.top()->Draw(mCommandList.Get(), CurrentBackBufferView(), DepthStencilView(), CurrentBackBuffer(), mCurrBackBufferIndex);

	// ȭ�� ������ ���¸� �ٽ� PRESENT ���·� �����Ѵ�.
	mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	
	auto ui = mScenes.top()->GetUI();
	//if(ui) ui->Flush();

	// Ŀ�ǵ� ����Ʈ�� ��ɾ���� �� �����ϱ���� ��ٸ���.
	WaitUntilGPUComplete();

	ThrowIfFailed(mD3dDevice->GetDeviceRemovedReason());
	ThrowIfFailed(mSwapChain->Present(0, 0));  // ȭ����۸� Swap�Ѵ�.	
	
	// ���� �ĸ���� ��ġ�� �̵��� �� �ٽ� ��ٸ���.
	mCurrBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	
	WaitUntilGPUComplete();
}

