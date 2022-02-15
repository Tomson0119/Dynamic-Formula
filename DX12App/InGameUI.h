#pragma once
#include "UI.h"

class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    ~InGameUI();
    virtual void Update(float GTime, Player* mPlayer);
    void Draw(UINT nFrame);
    virtual void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    virtual void Reset();
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
    void CreateFontFormat();
    void SetVectorSize(UINT nFrame, UINT TextCnt);
    void SetTextRect();
    void StartPrint(const std::wstring& strUIText);
    void SetDraftGage();
    
private:
    virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);

    float mfWidth = 0.0f;
    float mfHeight = 0.0f;

    UINT uItemCnt = 0;
    float fDraftGage = 0.0f;    

    UINT TextCnt;
    UINT UICnt;

    //ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;

    //ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
    std::vector<TextBlock>          mvTextBlocks;
    //std::vector<std::wstring> TextUI;
};