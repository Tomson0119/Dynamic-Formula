#pragma once
#include "UI.h"

class LoginUI : public UI
{
public:
	LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
	virtual ~LoginUI() = default;
	virtual void Update(float GTime, std::vector<std::string>& Texts) override ;
	virtual void SetVectorSize(UINT nFrame) override ;
	virtual void Draw(UINT nFrame) override ;
	virtual void SetTextRect() override ;
	virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
		ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override ;
	virtual void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight) override;
	virtual void Reset() override ;
	virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) override;
	virtual int  OnProcessMouseClick(WPARAM buttonState, int x, int y) override;
	virtual void ChangeTextAlignment(UINT uNum, UINT uState) override;
	virtual std::pair<const std::string&, const std::string&> GetLoginPacket() { return std::make_pair(GetTextBlock()[3].strText, GetTextBlock()[4].strText); }

	void CreateFontFormat();
	bool MouseCollisionCheck(float x, float y, const TextBlock& TB);
	void SetLoginFail(bool TF) { mIsLoginFail = TF; }
	void SetSignupBool(bool TF) { mIsSignup = TF; }
	//void SetLoginOrSignup(std::string msg) { GetTextBlock()[1].strText = LoginOrSignup = msg; }
	//void SetLoginOrSignupReverse(std::string msg) { GetTextBlock()[5].strText = LoginOrSignupReverse = msg; }
private:
	bool mIsSignup = false;
	bool mIsLoginFail = true;

	std::vector<D2D1::ColorF> mColors;

	//std::string mLoginOrSignup = "Login";
	//std::string mLoginOrSignupReverse = "Signup";

	//For Packet
	//ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};

