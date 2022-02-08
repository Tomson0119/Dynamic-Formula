#pragma once
#include "UI.h"

class LoginUI : public UI
{
	LoginUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue*
		pd3dCommandQueue);
	~LoginUI();
};

