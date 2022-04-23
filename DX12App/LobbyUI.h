#pragma once
#include "UI.h"
class LobbyUI : public UI
{
public:
	LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
		pd3dCommandQueue);
	~LobbyUI();

	void SetVectorSize(UINT nFrame);
	virtual void Update(float GTime, Player* mPlayer) {}
	virtual void Update(float GTime);
	void Draw(UINT nFrame);
	void CreateFontFormat();
	void SetTextRect();
	void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
	void Reset();
	void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y);
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y);
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y);

	bool LobbyUI::MouseCollisionCheck(float x, float y, const TextBlock& TB);

private:
	virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

