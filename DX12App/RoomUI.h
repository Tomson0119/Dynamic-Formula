#pragma once
#include "UI.h"

struct PlayerData
{
	std::string Nickname;
	uint8_t color;
	bool IsEmpty;
	bool IsReady;
	PlayerData() {}
};

class NetModule;

class RoomUI : public UI
{
public:
	RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue *pd3dCommandQueue, NetModule& netRef);
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

	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y) override;
	virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y)  override;
	virtual void OnProcessMouseDown(WPARAM btnState, int x, int y) override;

	virtual void SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue,
		ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) override;
	void SetIndexPlayerInfo(int index, char* name, uint8_t color, bool empty, bool ready);
	const std::array<PlayerData, 8>& GetPlayerDatas() const { return mPlayerDatas; }
	bool MouseCollisionCheck(float x, float y, const TextBlock& TB);
	void CreateFontFormat();
	void SetStateFail(int result);
	void SetStateNotFail();
	//For Packet
	// BackgroundCarVisible, BackgoundCarInvisible
	void SetIndexCar(int index) { BitmapOpacities[index + 1] = 1.0f; }
	void SetIndexCarInvisible(int index) { BitmapOpacities[index + 1] = 0.0f; }
	// NicknameVisible, NicknameInvisible
	void SetIndexNicknameVisible(int index) { SetIndexColor(index + 3, D2D1::ColorF(D2D1::ColorF::LightGray, 1.0f)); }
	void SetIndexNicknameInvisible(int index) { SetIndexColor(index + 3, D2D1::ColorF(D2D1::ColorF::LightGray, 0.0f)); }
	// BackgroundColorVisible, BackgroundColorInvisible
	void SetIndexBackGroundVisible(int index);
	void SetIndexBackGroundInvisible(int index);
	//SetIsAdmin else NotAdmin
	void SetIndexIsAdmin(int index);
	// VisibleState, InvisibleState
	void SetIndexInvisibleState(int index);
	void SetIndexVisibleState(int index);
	//SetNickname
	void SetPlayerNickName(int index, const char* name) { mPlayerDatas[index].Nickname = name; }
	//Ready Visible, Invisible
	void SetIndexReady(int index);
	void SetIndexNotReady(int index);

	void SetIndexReadyOrAdminText();
	//RoomID
	void SetRoomID(int roomid) { mRoomID = roomid; }
	//MapID
	void SetMapID(int mapid) { mMapID = mapid; }
	//MyID
	void SetMyIndex(int clientid) { mMyIndex = clientid; }
	//AllPlayerSet
	void SetAllPlayerState();

private:
	float BitmapOpacities[9] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	
	NetModule& mNetRef;
	//std::array<bool, 8> mIsInvisible;
	
	std::array<std::string, 8> mReadyOrAdmin;
	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

