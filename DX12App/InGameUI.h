#pragma once
#include "UI.h"
constexpr float START_DELAY_TIME = 47.5f;
class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    virtual ~InGameUI() = default;
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
    void StartPrint(const std::string& strUIText);
    std::vector<XMFLOAT4>& GetLTRB() { return LTRB; }
    void SetLTRB(const std::vector<XMFLOAT4>& Rects) { int i = 0;  for (auto& R : Rects) LTRB.push_back(R); }

    //For Packet
    //void UpdateItemCnt(bool IsPlus) { if (IsPlus&&muItemCnt<2) ++muItemCnt; else if(!IsPlus && muItemCnt>0) --muItemCnt; }
    void UpdateScore() {}
    void UpdateRank() {}
    void UpdateIngameTime() {}

    void SetDriftGauge(int gauge) { mDriftGauge = gauge; }
    void SetRunningTime(float time) { mRunningTime = time; }

private:
    virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

    float mfWidth = 0.0f;
    float mfHeight = 0.0f;

    float mRunningTime;

    std::atomic_int mItemCnt = 0;
    std::atomic_int mDriftGauge = 0;

    float AnimEndTime = 0.0f;
    float fOpacities[7] = { 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f };

    bool mbIsStartUI[4] = { false, false, false, false };

    XMFLOAT2 mf2StartUIPos = { GetFrameWidth() * 0.5f, GetFrameHeight() * 0.5f };
    
    std::vector<float> mfOpacities;
    std::vector<XMFLOAT4> LTRB;
    std::vector<TextBlock>          mvTextBlocks;

    //For Packet
    std::array<UINT, 8> mauScores;
    std::array<UINT, 8> mauRanks;
    float mfVelocity;
    bool mbIsReverse = false;
    float mfIngameTime;

    //ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};