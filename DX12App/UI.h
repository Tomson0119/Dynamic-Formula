#pragma once
#include "player.h"

struct TextBlock
{
    std::string        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat* pdwFormat;
};

struct GradientColors
{
    UINT ColorCnt;
    D2D1::ColorF* ColorList;
};

class UI 
{
public:
    explicit UI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    ~UI();
    virtual void Update(float GTime, Player* mPlayer) {};
    virtual void Update(float GTime, std::vector<std::wstring>& Texts) {}
    virtual void Update(std::vector<std::wstring>& Texts) {}
    virtual void Update(float GTime);
    virtual void Draw(UINT nFrame/*, UINT TextCnt, UINT GradientCnt, const std::vector<TextBlock> &mvTextBlocks,
     XMFLOAT4 RetLTRM[], XMFLOAT4 FillLTRB[]*/);

    virtual HRESULT LoadBitmapResourceFromFile(PCWSTR ImageName, int index);
    virtual void DrawBmp(XMFLOAT4 RectLTRB[], UINT StartNum, UINT BmpNum, const float aOpacities[]);

    virtual void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    virtual void BuildBrush(UINT UICnt, UINT TextCnt, D2D1::ColorF* ColorList, 
        UINT gradientCnt, D2D1::ColorF* gradientColors);

    virtual void BuildSolidBrush(UINT UICnt, UINT TextCnt, D2D1::ColorF* ColorList);
    virtual void BuildSolidBrush(UINT UICnt, UINT TextCnt, std::vector<D2D1::ColorF>& ColorList);
    virtual void BuildLinearGradientBrush(UINT ColorCnt, D2D1::ColorF* ColorList);
    virtual void BuildLinearGradientBrushes(UINT GradientCnt, GradientColors GColors[]);
    virtual void Reset();
    virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
    virtual void SetVectorSize(UINT nFrame);
    virtual void CreateFontFormat(float vFontSize, const std::vector<std::wstring>& Fonts, UINT TextCnt,
        DWRITE_TEXT_ALIGNMENT* Alignment);
    virtual void CreateFontFormat(std::vector<float>& vFontSize, const std::vector<std::wstring>& Fonts, UINT TextCnt,
        DWRITE_TEXT_ALIGNMENT* Alignment);

    //ID2D1DeviceContext2* GetDeviceContext() { return mpd2dDeviceContext.Get(); }
    virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) {}
    virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) {}
    virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) {  }
    virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y) { return 0; }
    void BeginDraw(UINT nFrame);
    void TextDraw(UINT nFrame, UINT TextCnt, const std::vector<TextBlock> &mvTextBlocks);
    void RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT TextCnt, UINT bias, UINT GradientCnt, bool IsOutlined[]);
    void RoundedRectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT TextCnt, UINT bias, UINT GradientCnt, bool IsOutlined[]);
    void RoundedRectDraw() {}
    void EndDraw(UINT nFrame);
    void Flush();

    virtual void ChangeTextAlignment(UINT uNum, UINT uState) {}


    //void SetTextcnt(UINT Cnt) { TextCnt = Cnt; }
    std::pair<float, float> GetFrame() const { return std::make_pair(mfWidth, mfHeight); }
   ID3D11Resource* GetRenderTarget() const { return mvWrappedRenderTargets[0].Get(); }
   UINT GetRenderTargetsCount() const { return static_cast<UINT>(mvWrappedRenderTargets.size()); }

   virtual std::pair<const std::string&, const std::string&> GetLoginPacket() { return std::make_pair("", ""); }
   virtual int GetLobbyPacket() { return -1; }
   virtual int GetRoomPacket() { return -1; }
   //virtual std::vector<ComPtr<ID2D1Bitmap>> GetBitmaps() { return mvBitmaps; }
   
   void SetBitmapsSize(int size) { mvBitmaps.resize(size); }

   void SetTextCnt(int TextCnt) { miTextCnt = TextCnt; }
   void SetRectCnt(int RectCnt) { miRectCnt = RectCnt; }
   void SetRoundedRectCnt(int RoundeRectCnt) { miRoundRectCnt = RoundeRectCnt; }
   void SetEllipseCnt(int EllipseCnt) { miEllipseCnt = EllipseCnt; }
   void SetGradientCnt(int GradientCnt) { miGradientCnt = GradientCnt; }
   void SetBitmapCnt(int BitmapCnt) { miBitmapCnt = BitmapCnt; }

   void SetFrame(float H, float W) { mfHeight = H; mfWidth = W; }
   void SetBitmapFileNames(const std::vector<PCWSTR>& names) { for (auto& name : names) mvBitmapFileNames.push_back(name); }
private:

    float mfHeight = 0.0f;
    float mfWidth = 0.0f;

    //Text and UI Count
    UINT miTextCnt = 0;
    UINT miRectCnt = 0;
    UINT miRoundRectCnt = 0;
    UINT miEllipseCnt = 0;
    UINT miGradientCnt = 0;
    UINT miBitmapCnt = 0;
    
    GradientColors* pGradientColors;

    std::vector<PCWSTR> mvBitmapFileNames;


    ComPtr<ID3D11DeviceContext> mpd3d11DeviceContext;
    ComPtr<ID3D11On12Device> mpd3d11On12Device;
    ComPtr<ID2D1Factory3> mpd2dFactory;
    ComPtr<IDWriteFactory> mpd2dWriteFactory;
    ComPtr<ID2D1Device2> mpd2dDevice;
    ComPtr<ID3D11Device> pd3d11Device;
    ComPtr<IDXGIDevice> pdxgiDevice;
    ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;

    IWICImagingFactory* mWICFactoryPtr;

    std::vector<ComPtr<ID2D1Bitmap>> mvBitmaps;

    std::vector<ComPtr<IDWriteTextFormat>> mvdwTextFormat;

    std::vector<ComPtr<ID2D1SolidColorBrush>> mvd2dSolidBrush;
    ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
    std::vector<ComPtr<ID3D11Resource>>    mvWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      mvd2dRenderTargets;
};