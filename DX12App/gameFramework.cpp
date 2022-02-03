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

	// 초기화하는 명령어를 넣기 위해 커맨드 리스트를 개방한다.
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	//mScenes.push(make_unique<InGameScene>());
	mScenes.top()->BuildObjects(mD3dDevice.Get(), mCommandList.Get(), GetAspect(), mBtDynamicsWorld);
	
	//InGameUI Build
	mpInGameUI = std::make_unique<InGameUI>(mSwapChainBufferCount, mD3dDevice.Get(), mCommandQueue.Get());
	mpInGameUI->PreDraw(mSwapChainBuffers->GetAddressOf(), gFrameWidth, gFrameHeight);

	// Command List를 닫고 Queue에 명령어를 싣는다.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	// 초기화가 진행될 때까지 기다린다.
	WaitUntilGPUComplete();

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
	//if (mpInGameUI.get())
		//mpInGameUI.get()->Reset();
	D3DFramework::OnResize();
	//mpUI.get()->Initialize(mD3dDevice.Get(), mCommandQueue.Get());
	//if(mpInGameUI.get())
		//mpInGameUI.get()->OnResize(mSwapChainBuffers->GetAddressOf(),  mD3dDevice.Get(), mCommandQueue.Get(), mSwapChainBufferCount, gFrameWidth, gFrameHeight);
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

void GameFramework::TextUIUpdate()
{
	//const float fFontSize = m_fHeight / 25.0f;
	//const float fSmallFontSize = m_fHeight / 40.0f;

	//mpUI.get(). m_pd2dWriteFactory->CreateTextFormat(L"휴먼둥근헤드라인", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);

	//ThrowIfFailed(m_pdwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	//ThrowIfFailed(m_pdwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
	////TextUI Set
	TextUI.clear();
	TextUI.resize(5);
	
	//StartTime Set
	UINT Countdown = 3;
	float CountdownTime = 5.0f;
	if (mTimer.TotalTime() > 4.0f) 
	{
		for (auto wc : L"Start")
			TextUI[4].push_back(wc);
	}
	else if (mTimer.TotalTime() > 3.0f)
	{
		TextUI[4].push_back('0'+Countdown - 2);
	}
	else if (mTimer.TotalTime() > 2.0f)
	{
		TextUI[4].push_back('0' + Countdown - 1);
	}
	else if (mTimer.TotalTime() > 1.0f)
	{
		TextUI[4].push_back('0' + Countdown);
	}

	
	//Time Set
	if (mTimer.TotalTime() < CountdownTime)
		return;
	TextUI[4].clear();

	float LapTime = mTimer.TotalTime() - CountdownTime;
	UINT Min = 0;
	float Sec = 0.0;
	Min = LapTime / 60.0f;
	Sec = LapTime - (Min * 60.0f);

	if (Min < 10)
		TextUI[0].push_back('0');
	for (auto wc : std::to_wstring(Min))
		TextUI[0].push_back(wc);
	
	TextUI[0].push_back(':');
	if (Sec < 10)
		TextUI[0].push_back('0');
	for (int i = 0; i < 3+!(Sec<10); ++i)
		TextUI[0].push_back(std::to_wstring(Sec)[i]);
	
	//Lap Count Set
	if(static_cast<int>(mTimer.TotalTime()/60)>0)
	{
		for (auto wc : std::to_wstring(static_cast<int>(LapTime / 60)))
			TextUI[1].push_back(wc);
		for (auto wc : std::wstring{ L"Lap" })
			TextUI[1].push_back(wc);
	}
	//My Rank
	UINT MyRank = 1;
	TextUI[2].push_back(('0' + MyRank));
	
	switch (MyRank % 10)
	{
	case 1:
		TextUI[2].push_back('s');
		TextUI[2].push_back('t');
		break;
	case 2:
		TextUI[2].push_back('n');
		TextUI[2].push_back('d');
		break;
	case 3:
		TextUI[2].push_back('r');
		TextUI[2].push_back('d');
		break;
	default:
		TextUI[2].push_back('t');
		TextUI[2].push_back('h');
		break;
	}
	//Speed
	//float CurrentSpeed = mScenes.top().get()->GetPlayer()->GetCurrentVelocity();
	/*if (mScenes.top().get()->GetPlayer()->GetCurrentVelocity() >= 1000.0f)
	{
		for (int i = 0; i < 6; ++i)
			TextUI[3].push_back(to_wstring(mScenes.top().get()->GetPlayer()->GetCurrentVelocity())[i]);
	}
	else if (mScenes.top().get()->GetPlayer()->GetCurrentVelocity() >= 100.0f)
	{
		for (int i = 0; i < 5; ++i)
			TextUI[3].push_back(to_wstring(mScenes.top().get()->GetPlayer()->GetCurrentVelocity())[i]);
	}
	else if (mScenes.top().get()->GetPlayer()->GetCurrentVelocity() >= 10.0f)
	{
		for (int i = 0; i < 4; ++i)
			TextUI[3].push_back(to_wstring(mScenes.top().get()->GetPlayer()->GetCurrentVelocity())[i]);
	}
	else
	{
		for (int i = 0; i < 3; ++i)
			TextUI[3].push_back(to_wstring(0.0f)[i]);
	}*/
	for(auto wc : std::wstring(L"km/h"))
		TextUI[3].push_back(wc);



	//TextUI[4].push_back();
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
	TextUIUpdate();
	mpInGameUI->Update(TextUI);
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
	/*mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));*/
	/*mCommandList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), 
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST));*/
	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	//mpUI->Resize(mSwapChainBuffers->GetAddressOf(), gFrameWidth, gFrameHeight);
	
	mpInGameUI->Draw(mCurrBackBufferIndex);

	// 커맨드 리스트의 명령어들을 다 실행하기까지 기다린다.
	WaitUntilGPUComplete();
	//mpUI.get()->GetD3D11DeviceContext()->Flush();
	

	ThrowIfFailed(mD3dDevice->GetDeviceRemovedReason());
	ThrowIfFailed(mSwapChain->Present(0, 0));  // 화면버퍼를 Swap한다.	

	
	// 다음 후면버퍼 위치로 이동한 후 다시 기다린다.
	mCurrBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	
	WaitUntilGPUComplete();
}

