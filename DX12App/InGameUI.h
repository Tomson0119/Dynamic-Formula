#pragma once
#include "UI.h"
constexpr float START_DELAY_TIME = 47.5f;
class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    ~InGameUI();
    void Update(float GTime, Player* mPlayer) override ;
    void SetVectorSize(UINT nFrame) override;
    void Draw(UINT nFrame) override;
    virtual void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) override;
    virtual void Reset() override;
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override;
    void SetTextRect() override;
    virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
    virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
    void CreateFontFormat();
    void StartAnimation(float GTime);
    void SetDraftGage();
    void StartPrint(const std::string& strUIText);
    std::vector<XMFLOAT4>& GetLTRB() { return LTRB; }
    void SetLTRB(const std::vector<XMFLOAT4>& Rects) { int i = 0;  for (auto& R : Rects) LTRB.push_back(R); }

private:
   void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue) override;

    UINT uItemCnt = 0;

    float fDriftGauge = 0.0f;    
    float mItemOffTime = 3.5f;
    float mItemOffStart = 0.0f;
    float mItemOffEnd = 0.0f;
    float AnimEndTime = 0.0f;
    float fOpacities[7] = { 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f };

    bool mItemOff = false;
    bool mIsShootingMissile = false;
    bool mIsBoost = false;
    bool mbIsStartUI[4] = { false, };

    XMFLOAT2 mf2StartUIPos = { GetFrameWidth() * 0.5f, GetFrameHeight() * 0.5f };
    
    std::vector<float> mfOpacities;
    std::vector<XMFLOAT4> LTRB;
    std::vector<TextBlock>          mvTextBlocks;

    //ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};