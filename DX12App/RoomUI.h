#pragma once
#include "UI.h"

class RoomUI : public UI
{
public:
	RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
		pd3dCommandQueue);
	virtual ~RoomUI();
	void Update(float GTime);
	void Update(float GTime, std::vector<std::string> Texts) {}
	void Update(float GTime, Player* mPlayer) {}
	void SetVectorSize(UINT nFrame) override;
	void Draw(UINT nFrame) override;
	void SetTextRect() override ;
	void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight) override;
	void Reset() override;
	void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override;
	void CreateFontFormat();
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y)  override { return 0; }

	//For Packet
	void SetStartOrReady();
	void UpdatePlayerState(int index, bool state) { mabIsInRoom[index] = state; }
	void UpdatePlayerNickName(int index, const std::string& name) { masNicknames[index] = name; }
private:
	void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue) override;

	//For Packet
	int miMyRoomID = -1;
	bool mbIsRoomMaster = false;
	bool mbIsReady = false;

	int miMapNum = -1;
	//std::array<D2D1::ColorF, 8> maColors;

	std::array<bool, 8> mabIsInRoom;
	std::array<std::string, 8> masNicknames;

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

