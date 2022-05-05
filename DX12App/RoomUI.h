#pragma once
#include "UI.h"

class RoomUI : public UI
{
public:
	RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue *pd3dCommandQueue);
	virtual ~RoomUI() = default;
	virtual void Update(float GTime);
	virtual void Update(float GTime, std::vector<std::string> Texts) {}
	virtual void Update(float GTime, Player* mPlayer) {}
	virtual void SetVectorSize(UINT nFrame) override;
	virtual void Draw(UINT nFrame) override;
	virtual void SetTextRect() override ;
	virtual void BuildObjects(ID3D12Resource**ppd3dRenderTargets, UINT Width, UINT Height) override;
	virtual void Reset() override;
	virtual void OnResize(ID3D12Resource **ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override;
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y)  override { return 0; }
	virtual void SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue,
		ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) override;
	void CreateFontFormat();
	//For Packet
	void SetAllPlayerNickNames(std::array<std::string, 8>& names);
	void SetStartOrReady();
	void SetPlayerState(int index, bool state) { mIsInRooms[index] = state; }
	void SetPlayerNickName(int index, const std::string& name) { mNicknames[index] = name; }
	void SetPlayerAdmin(std::array<bool, 8> IsAdmins);
private:
	//For Packet
	int mMyRoomID;
	std::array<bool, 8> mIsAdmin;
	std::array<bool, 8> mIsInRooms;
	std::array<std::string, 8> mNicknames;
	bool mIsReady = false;

	//int mMapNum;
	//int mCarNum;
	//std::array<D2D1::ColorF, 8> maColors;

	

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

