#pragma once
#include "UI.h"
class LobbyUI : public UI
{
public:
	LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
		pd3dCommandQueue);
	virtual ~LobbyUI() = default;
	virtual void Update(float GTime, Player* mPlayer) {}
	virtual void Update(float GTime, std::vector<std::string> Texts) {}
	virtual void Update(float GTime);
	virtual void SetVectorSize(UINT nFrame) override ;
	virtual void Draw(UINT nFrame) override;
	virtual void SetTextRect() override ;
	virtual void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight) override;
	virtual void Reset() override;
	virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) override;
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y) override;
	virtual void SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue,
		ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) override;
	void CreateFontFormat();
	bool MouseCollisionCheck(float x, float y, const RECT& rc);
	RECT MakeRect(float left, float top, float right, float bottom);
	void SetRoomNums(int num, int index) { mRoomNums[index] = num; }
	void UpdateRoomNumsText();
private:
	std::array<int, 6> mRoomNums;
	float aOpacities[4] = { 0.5f, 1.0f, 0.7f, 0.7f };
	//std::vector<std::string>  RoomNum;

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

