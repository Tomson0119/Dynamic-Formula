#pragma once
#include "UI.h"

class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    ~InGameUI();
    virtual void Update(float GTime, Player* mPlayer);
    virtual void Update(float GTime, std::vector<std::string> Texts);
    void Draw(UINT nFrame);
    virtual void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    virtual void Reset();
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
    void CreateFontFormat();
    void SetVectorSize(UINT nFrame, UINT TextCnt);
    void SetTextRect();
    void StartPrint(const std::string& strUIText);
    void SetDraftGage();
    virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y);
    virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam);
private:
    virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

    UINT uItemCnt = 0;
    float fDriftGauge = 0.0f;    
    
    float mItemOffTime = 3.5f;
    float mItemOffStart = 0.0f;
    float mItemOffEnd = 0.0f;

    bool mItemOff = false;
    bool mIsShootingMissile = false;
    bool mIsBoost = false;

    //ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;

    //ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
    std::vector<TextBlock>          mvTextBlocks;
    //std::vector<std::wstring> TextUI;
};