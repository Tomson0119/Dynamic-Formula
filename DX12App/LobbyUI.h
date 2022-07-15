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

	void SetVisibleDenyBox();
	void SetInvisibleDenyBox();

	void SetIndexRoomNums(int index, int RoomID) { mRoomNums[index] = RoomID; }
	void RoomEmptyProcess();

	void UpdateRoomIDTextsIndex(int index, int RoomID, bool Opened);

	void UpdatePlayerCountTextsIndex(int index, int PlayerCount);

	void UpdateMapIDTextsIndex(int index, int MapID);

	void UpdateGameStartedTexts();
	void UpdateGameStartedTextsIndex(int index, bool IsGameStarted);

    
	void UpdateRoomIsOpenedIndex(int index, bool IsOpened) { mIsOpened[index] = IsOpened; }

	void SetRoomInfoTextsIndex(int index, int RoomID, unsigned char PlayerCount, unsigned char MapID, bool GameStarted, bool Opened);

	void SetRoomInfo(int index, int RoomID, unsigned char PlayerCount, unsigned char MapID, bool GameStarted, bool Opened);
private:
	float aOpacities[4] = { 0.5f, 1.0f, 0.7f, 0.7f };
	bool mIsMouseCollisionRoom[6] = {false, false, false, false, false ,false};
	bool mIsMouseCollisionMakeRect{};
	bool mIsMouseCollisionLeftArrow{};
	bool mIsMouseCollisionRightArrow{};
	//For Packet
	// º¸·ù

	
	std::atomic_char mDenyMessageCode;
	std::atomic_bool mIsDenyBox = false;

	//Room Info
	std::atomic_int mRoomNums[6] = {-1, -1, -1, -1, -1, -1};
	/*std::atomic_char mPlayerCount[6];
	std::atomic_char mMapID[6];*/
	std::atomic_bool mIsGameStarted[6] = {false, false, false, false, false, false};
	std::atomic_bool mIsOpened[6] = {false, false, false, false, false, false };

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

