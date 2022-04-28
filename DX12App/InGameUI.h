#pragma once
#include "UI.h"

class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    virtual ~InGameUI() = default;
    virtual void Update(float GTime, Player* mPlayer);
    virtual void Update(float GTime, std::vector<std::string> Texts);
    virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y);
    virtual void Reset();
    virtual void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam);
    
    void Draw(UINT nFrame);
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
    void CreateFontFormat();
    void SetVectorSize(UINT nFrame, UINT TextCnt);
    void SetTextRect();
    void StartPrint(const std::wstring& strUIText);

    void SetDriftGauge(int gauge);
    void SetRunningTime(float time) { mRunningTime = time; }

private:
    virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

    float mfWidth = 0.0f;
    float mfHeight = 0.0f;

    std::atomic_int mItemCnt = 0;
    std::atomic_int mDriftGauge = 0;

    UINT TextCnt;
    UINT UICnt;
    
    float mItemOffTime = 3.5f;
    float mItemOffStart = 0.0f;
    float mItemOffEnd = 0.0f;

    bool mItemOff = false;
    bool mIsShootingMissile = false;
    bool mIsBoost = false;

    float mRunningTime;

    //ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;

    //ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
    std::vector<TextBlock>          mvTextBlocks;
    //std::vector<std::wstring> TextUI;
};