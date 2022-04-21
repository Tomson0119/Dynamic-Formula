#pragma once
#include "UI.h"

class RoomUI : public UI
{
public:
	RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
		pd3dCommandQueue);
	~RoomUI();
	void SetVectorSize(UINT nFrame, UINT TextCnt);
	virtual void Update(float GTime);
	virtual void Update(float GTime, Player* mPlayer) {}
	void Draw(UINT nFrame);
	void CreateFontFormat();
	void SetTextRect();
	void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
	void Reset();
	void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y) { return 0; }

private:
	virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

	float mfWidth = 0.0f;
	float mfHeight = 0.0f;

	UINT TextCnt;
	UINT UICnt;

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
	std::vector<TextBlock>          mvTextBlocks;

	std::vector<PCWSTR > mvBitmapFileNames;

};

