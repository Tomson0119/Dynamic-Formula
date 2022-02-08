#pragma once
#include "UI.h"

class RoomUI : public UI
{
	RoomUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue*
		pd3dCommandQueue);
	~RoomUI();
};

