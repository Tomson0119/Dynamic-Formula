#pragma once
#include "player.h"
struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat* pdwFormat;
};
class UI 
{
public:
    explicit UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    ~UI();
    virtual void Update(float GTime, Player* mPlayer) = 0;
    virtual void Update(float GTime);
    virtual void Draw(UINT nFrame/*, UINT TextCnt, UINT GradientCnt, const std::vector<TextBlock> &mvTextBlocks,
     XMFLOAT4 RetLTRM[], XMFLOAT4 FillLTRB[]*/);
    virtual void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    virtual void BuildBrush(UINT UICnt, UINT TextCnt, D2D1::ColorF* ColorList, 
        UINT gradientCnt, D2D1::ColorF* gradientColors);
    virtual void BuildSolidBrush(UINT UICnt, UINT TextCnt, D2D1::ColorF* ColorList);
    virtual void BuildLinearGradientBrush(UINT ColorCnt, D2D1::ColorF* ColorList);
    virtual void Reset();
    virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
    virtual void SetVectorSize(UINT nFrame);
    virtual void CreateFontFormat(float FontSize, const std::vector<std::wstring> &Fonts, UINT TextCnt,
        DWRITE_TEXT_ALIGNMENT Alignment = DWRITE_TEXT_ALIGNMENT_CENTER);

    //ID2D1DeviceContext2* GetDeviceContext() { return mpd2dDeviceContext.Get(); }

    void BeginDraw(UINT nFrame);
    void TextDraw(UINT nFrame, UINT TextCnt, const std::vector<TextBlock> &mvTextBlocks);
    void RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT TextCnt, UINT bias, UINT GradientCnt);
    void RoundedRectDraw() {}
    void EndDraw(UINT nFrame);
    void Flush();

    //void SetTextcnt(UINT Cnt) { TextCnt = Cnt; }
    std::pair<float, float> GetFrame() const { return std::make_pair(mfWidth, mfHeight); }
   ID3D11Resource* GetRenderTarget() const { return mvWrappedRenderTargets[0].Get(); }
   UINT GetRenderTargetsCount() const { return mvWrappedRenderTargets.size(); }
private:
    virtual void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float mfHeight = 0.0f;
    float mfWidth = 0.0f;

    //UINT TextCnt = 0;

    ComPtr<ID3D11DeviceContext> mpd3d11DeviceContext;
    ComPtr<ID3D11On12Device> mpd3d11On12Device;
    ComPtr<ID2D1Factory3> mpd2dFactory;
    ComPtr<IDWriteFactory> mpd2dWriteFactory;
    ComPtr<ID2D1Device2> mpd2dDevice;
    ComPtr<ID3D11Device> pd3d11Device;
    ComPtr<IDXGIDevice> pdxgiDevice;
    ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;

    std::vector<ComPtr<IDWriteTextFormat>> mvdwTextFormat;

    std::vector<ComPtr<ID2D1SolidColorBrush>> mvd2dSolidBrush;
    ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
    std::vector<ComPtr<ID3D11Resource>>    mvWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      mvd2dRenderTargets;
};



