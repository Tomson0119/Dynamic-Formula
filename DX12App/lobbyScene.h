#pragma once

#include "scene.h"

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
	LobbyScene(NetModule* netPtr);
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
		std::shared_ptr<BulletWrapper> physics) override;

	virtual void Update(
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		std::shared_ptr<BulletWrapper> physics) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE backBufferview, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, ID3D12Resource* backBuffer, UINT nFrame) override;

	virtual bool ProcessPacket(std::byte* packet, char type, int bytes) override;

private:
	

private:
	std::unordered_map<int, Room> mRoomList;
};