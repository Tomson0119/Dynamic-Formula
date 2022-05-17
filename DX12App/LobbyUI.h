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
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) override;
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y) override;
	virtual void SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue,
		ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) override;
	void CreateFontFormat();
	
	//For Packet
	bool MouseCollisionCheck(float x, float y, const RECT& rc);
	void RoomMouseCheck(float dx, float dy, float left, float top, float right, float bottom, int index);
	RECT MakeRect(float left, float top, float right, float bottom);
	void UpdateDenyBoxText();
	void SetDenyTextCode(char code) { mDenyMessageCode = code; }
	void SetDenyBox() { mIsDenyBox = true; }
	void SetIndexRoomNums(int index, int RoomID) { mRoomNums[index] = RoomID; }
	void UpdateRoomIDTexts();
	void RoomEmptyProcess();
private:
	float aOpacities[4] = { 0.5f, 1.0f, 0.7f, 0.7f };
	//For Packet
	// º¸·ù
	std::atomic_int mRoomNums[6];
	std::atomic_char mDenyMessageCode;
	std::atomic_bool mIsDenyBox = false;

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

