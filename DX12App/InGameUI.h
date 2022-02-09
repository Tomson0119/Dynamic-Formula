#pragma once
#include "UI.h"

class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    ~InGameUI();
    void Update(const std::vector<std::wstring>& strUIText);
    void Draw(UINT nFrame);
    void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    void BuildBrush(UINT UI_Cnt, D2D1::ColorF* ColorList, UINT gradientCnt, D2D1::ColorF* gradientColors);
    void BuildSolidBrush(UINT UI_Cnt, D2D1::ColorF* ColorList);
    void BuildLinearGradientBrush(UINT gradientCnt, D2D1::ColorF* gradientColors);
    void Reset();
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);

    void CreateFontFormat();
    void SetVectorSize(UINT nFrame, UINT TextCnt);
    void SetTextRect();
    void StartPrint(const std::wstring& strUIText);
    void SetDraftGage();
    void Flush();
    
private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(mvWrappedRenderTargets.size()); }
    virtual void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float mfWidth = 0.0f;
    float mfHeight = 0.0f;

    float GradientPoint = 0.0f;

    UINT uItemCnt = 0;
    float fDraftGage = 0.0f;

    POINT DraftUIStart;
    POINT DraftUIEnd;

    UINT TextCnt;
    UINT UICnt;

    ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;

    std::vector<ComPtr<ID2D1SolidColorBrush>> mvd2dTextBrush;

    std::vector<ComPtr<ID3D11Resource>>    mvWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      mvd2dRenderTargets;
    std::vector<TextBlock>          mvTextBlocks;
};