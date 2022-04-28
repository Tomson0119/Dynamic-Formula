#pragma once
#include "UI.h"

class LoginUI : public UI
{
public:
	LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
	~LoginUI();
	void Update(float GTime, std::vector<std::string>& Texts) override ;
	void SetVectorSize(UINT nFrame) override ;
	void Draw(UINT nFrame) override ;
	void SetTextRect() override ;
	void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override ;
	void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight) override ;
	void Reset() override ;
	void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override ;
	void OnProcessMouseMove(WPARAM buttonState, int x, int y) override ;
	void OnProcessMouseDown(WPARAM buttonState, int x, int y) override ;
	int OnProcessMouseClick(WPARAM buttonState, int x, int y) override ;
	void ChangeTextAlignment(UINT uNum, UINT uState) override;
	void CreateFontFormat();
	bool MouseCollisionCheck(float x, float y, const TextBlock& TB);
	std::pair<const std::string&, const std::string&> GetLoginPacket() { return std::make_pair(GetTextBlock()[3].strText, GetTextBlock()[4].strText); }
	void SetLoginFail(bool TF) { IsLoginFail = TF; }
	void SetSignupBool(bool TF) { IsSignup = TF; }
	void SetLoginOrSignup(std::string msg) { GetTextBlock()[1].strText = LoginOrSignup = msg; }
	void SetLoginOrSignupReverse(std::string msg) { GetTextBlock()[5].strText = LoginOrSignupReverse = msg; }
private:
	void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue) override ;

	bool IsSignup = false;
	bool IsLoginFail = true;

	std::vector<D2D1::ColorF> mvColors;

	std::string LoginOrSignup = "Login";
	std::string LoginOrSignupReverse = "Signup";

	//For Packet
	

	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

