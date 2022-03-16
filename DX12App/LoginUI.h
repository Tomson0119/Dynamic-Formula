#pragma once
#include "UI.h"

class LoginUI : public UI
{
public:
	LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
		pd3dCommandQueue);
	~LoginUI();
	void SetVectorSize(UINT nFrame, UINT TextCnt);
	virtual void Update(float GTime, std::vector<std::wstring>& Texts);
	virtual void Update(float GTime) {}
	virtual void Update(std::vector<std::wstring>& Texts);
	virtual void Draw(UINT nFrame);
	void CreateFontFormat();
	void SetTextRect();
	virtual void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
	virtual void Reset();
	virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y);
	virtual char OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y);
	
	bool MouseCollisionCheck(float x, float y, const TextBlock& TB);
	virtual void ChangeTextAlignment(UINT uNum, UINT uState  );

	virtual std::pair<const std::string&, const std::string&> GetLoginPacket() { return std::make_pair(mvTextBlocks[3].strText, mvTextBlocks[4].strText); }

private:
	virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

	float mfWidth = 0.0f;
	float mfHeight = 0.0f;

	float fFontSize = 0.0f; 
	std::vector<std::wstring> Fonts;
	DWRITE_TEXT_ALIGNMENT TextAlignments[6];
	UINT TextCnt;
	UINT UICnt;
	std::vector<D2D1::ColorF> mvColors;
	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
	std::vector<TextBlock>          mvTextBlocks;
};

