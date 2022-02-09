#pragma once
#include "UI.h"

class LoginUI : public UI
{
public:
	LoginUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue*
		pd3dCommandQueue);
	~LoginUI();
	void SetVectorSize(UINT nFrame, UINT TextCnt);
	void Update(float GTime);
	void Draw(UINT nFrame);
	void CreateFontFormat();
	void SetTextRect();
	void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
	void Reset();
	void OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
private:
	virtual void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

	float mfWidth = 0.0f;
	float mfHeight = 0.0f;

	UINT TextCnt;
	UINT UICnt;

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
	std::vector<TextBlock>          mvTextBlocks;
};

