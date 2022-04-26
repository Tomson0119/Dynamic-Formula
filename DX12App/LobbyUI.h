#pragma once
#include "UI.h"
class LobbyUI : public UI
{
public:
	LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
		pd3dCommandQueue);
	~LobbyUI();
	void Update(float GTime, Player* mPlayer) {}
	void Update(float GTime, std::vector<std::string> Texts) {}
	void Update(float GTime);
	void SetVectorSize(UINT nFrame) override ;
	void Draw(UINT nFrame) override;
	void SetTextRect() override ;
	void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight) override;
	void Reset() override;
	void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override;
	void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	void OnProcessMouseDown(WPARAM buttonState, int x, int y) override;
	int OnProcessMouseClick(WPARAM buttonState, int x, int y) override;
	void CreateFontFormat();
	bool MouseCollisionCheck(float x, float y, const TextBlock& TB);
private:
	void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue) override;

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

