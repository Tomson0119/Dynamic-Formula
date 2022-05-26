#pragma once

#include "scene.h"
#include "lobbyUI.h"

struct Room
{
	int ID;
	unsigned char PlayerCount;
	unsigned char MapID;
	bool GameStarted;
	bool Closed;
};

class NetModule;

class LobbyScene : public Scene
{
public:
	LobbyScene(HWND hwnd, NetModule* netPtr);
	virtual ~LobbyScene() = default;

public:
	virtual void BuildObjects(
		ComPtr<ID3D12Device> device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12CommandQueue* cmdQueue,
		UINT nFrame,
		ID3D12Resource** backBuffer,
		float Width,
		float Height,
		float aspect,
		const std::shared_ptr<BulletWrapper>& physics) override;
	virtual void ProcessAfterResize();
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseDown(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseUp(WPARAM btnState, int x, int y);
	virtual void OnProcessMouseMove(WPARAM btnState, int x, int y);
	virtual void Update(
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		const std::shared_ptr<BulletWrapper>& physics) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, ID3D12Resource* depthBuffer, UINT nFrame) override;
	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;
	virtual void Reset() override;

	virtual UI* GetUI() const override { return mpUI.get(); }
	const std::array<Room, ROOM_NUM_PER_PAGE>& GetRoomList() const { return mRoomList; }

private:
	std::array<Room, ROOM_NUM_PER_PAGE> mRoomList;
	std::mutex mRoomListMut;

	std::unique_ptr<LobbyUI> mpUI;
	std::atomic_int mPageNum;
};