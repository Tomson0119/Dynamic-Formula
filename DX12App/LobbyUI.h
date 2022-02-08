#pragma once
#include "UI.h"
class LobbyUI : public UI
{
	LobbyUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue*
		pd3dCommandQueue);
	~LobbyUI();
};

