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
    virtual void Update(float GTime, Player* mPlayer) {}
    virtual void Update(float GTime, std::vector<std::string>& Texts) {} 
    virtual void Update(std::vector<std::string>& Texts) {}
    virtual void Update(float GTime) {}
    virtual void Draw(UINT nFrame);

    virtual HRESULT LoadBitmapResourceFromFile(std::wstring ImageName, int index);
    
    virtual void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    virtual void BuildBrush(D2D1::ColorF* ColorList, UINT gradientCnt, D2D1::ColorF* gradientColors);
    virtual void BuildBrush(std::vector<D2D1::ColorF>& ColorList, UINT gradientCnt, D2D1::ColorF* gradientColors);
    virtual void BuildSolidBrush(D2D1::ColorF* ColorList);
    virtual void BuildSolidBrush(const std::vector<D2D1::ColorF>& ColorList);
    virtual void BuildLinearGradientBrush(UINT ColorCnt, D2D1::ColorF* ColorList);
    virtual void BuildLinearGradientBrushes(UINT GradientCnt, GradientColors GColors[]);
    virtual void Reset();
    virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);
    virtual void SetVectorSize(UINT nFrame);
    virtual void CreateFontFormat(float vFontSize, const std::vector<std::wstring>& Fonts, const std::vector<DWRITE_TEXT_ALIGNMENT>& Alignment);
    virtual void CreateFontFormat(std::vector<float>& vFontSize, const std::vector<std::wstring>& Fonts, const std::vector<DWRITE_TEXT_ALIGNMENT>& Alignment);
    virtual  void SetTextRect() {}

    virtual void Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) {}
    virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) {}
    virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) {  }
    virtual int OnProcessMouseClick(WPARAM buttonState, int x, int y) { return 0; }

    void BeginDraw(UINT nFrame);
    void DrawBmp(XMFLOAT4 RectLTRB[], UINT StartNum, UINT BmpNum, const float aOpacities[]);
    void DrawBmp(const std::vector<XMFLOAT4>& RectLTRB, UINT StartNum, UINT BmpNum, const float aOpacities[]);
    void TextDraw(const std::vector<TextBlock> &mvTextBlocks);
    void RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT GradientCnt, bool IsOutlined[]);
    void RoundedRectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT GradientCnt, bool IsOutlined[]);
    void EndDraw(UINT nFrame);
    void Flush();

    virtual void ChangeTextAlignment(UINT uNum, UINT uState) {}

   ID3D11Resource* GetRenderTarget() const { return mvWrappedRenderTargets[0].Get(); }
   UINT GetRenderTargetsCount() const { return static_cast<UINT>(mvWrappedRenderTargets.size()); }
   
   virtual std::pair<const std::string&, const std::string&> GetLoginPacket() { return std::make_pair("", ""); }
   virtual int GetLobbyPacket() { return -1; }
   virtual int GetRoomPacket() { return -1; }
   
   //Frame
   void SetFrame(float W, float H) { mfHeight = H; mfWidth = W; }
   std::pair<float, float> GetFrame() const { return std::make_pair(mfWidth, mfHeight); }
   float GetFrameWidth() { return mfWidth; }
   float GetFrameHeight() { return mfHeight; }

   //Font 
   TCHAR* GetFontName() { return mtcFontName; }
   void FontLoad(const std::vector<std::wstring>& FontFilePath);

   //TextCnt
   UINT GetTextCnt() { return miTextCnt; }
   void SetTextCnt(UINT n) { miTextCnt = n; }

   //RoundRectCnt
   UINT GetRoundRectCnt() { return miRoundRectCnt; }
   void SetRoundRectCnt(UINT n) { miRoundRectCnt = n; }

   // RectCnt
   UINT GetRectCnt() { return miRectCnt; }
   void SetRectCnt(UINT n) { miRectCnt = n; }

   //Ellipse
   UINT GetEllipseCnt() { return miEllipseCnt; }
   void SetEllipseCnt(UINT n) { miEllipseCnt  = n; }

   //Gradient
   UINT GetGradientCnt() { return miGradientCnt; }
   void SetGradientCnt(UINT n) { miGradientCnt = n; }

   // Bitmap
   UINT GetBitmapCnt() { return miBitmapCnt; }
   void SetBitmapCnt(UINT n) { miBitmapCnt = n; }
   void SetBitmapsSize(UINT size) { mvBitmaps.resize(size); }

  //Font Size
   void ResizeFontSize(UINT n) { mvfFontSizes.resize(n); }
   void SetFontSize(const std::vector<float>& vfFontsizes) { for (int i = 0; i < static_cast<int>(GetTextCnt());++i) mvfFontSizes[i] = vfFontsizes[i]; }
   void SetFontSize(float FontSize, int index) { mvfFontSizes[index] = FontSize; }
   float GetFontSize(int index) { return mvfFontSizes[index]; }
   std::vector<float>& GetFontSize() { return mvfFontSizes; }

   //Fonts
   void ResizeFonts(UINT n) { mvwsFonts.resize(n); }
   std::vector<std::wstring>& GetFonts() { return mvwsFonts; }
   void SetFonts(const std::vector<std::wstring>& Fonts) { for (int i = 0; i < static_cast<int>(GetTextCnt()); ++i) mvwsFonts[i] = Fonts[i]; }

   //BitmapFileName
   void ResizeBitmapNames(UINT n) { mvBitmapFileNames.resize(n); }
   std::vector<std::wstring>& GetBitmapFileNames() { return mvBitmapFileNames; }
   void SetBitmapFileNames(const std::vector<std::wstring>& names) { for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i) mvBitmapFileNames[i] = names[i]; }

   //TextBlock
   void ResizeTextBlock(UINT n) { mvTextBlocks.resize(n); }
   std::vector<TextBlock>& GetTextBlock() { return mvTextBlocks; }
   void SetTextBlock(const std::vector<TextBlock>& TextBlocks);

   //Colors - no resize
   std::vector<D2D1::ColorF>& GetColors() { return mvColors; }
   void SetColors(const std::vector<D2D1::ColorF>& Colors) { for (auto& color : Colors) mvColors.push_back(color); }
   void SetColors(D2D1::ColorF* Colors) { for (int i = 0; i < static_cast<int>(mvColors.size()); ++i) mvColors.push_back(Colors[i]); }

   //TextAllignments
   void ResizeTextAlignment(UINT n) { mvdwTextAlignments.resize(n); }
   std::vector<DWRITE_TEXT_ALIGNMENT>& GetTextAlignment() { return mvdwTextAlignments; }
   void SetTextAllignments(const std::vector< DWRITE_TEXT_ALIGNMENT>& Allignments) { for (int i = 0; i < static_cast<int>(GetTextCnt());++i) mvdwTextAlignments[i] = Allignments[i]; }
   
   //UI
   void SetUICnt() { miUICnt = miRectCnt + miRoundRectCnt + miEllipseCnt; }
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
    UINT miUICnt = 0;
    
    std::vector<float> mvfFontSizes; //FontSize
    std::vector<std::wstring> mvwsFonts; //Fonts
    std::vector<std::wstring> mvBitmapFileNames; //BitmapFileNames
    std::vector<ComPtr<ID2D1Bitmap1>> mvBitmaps; //Bitmaps
    std::vector<TextBlock>          mvTextBlocks; //TextBlocks
    std::vector<D2D1::ColorF> mvColors; //Colors
    std::vector<DWRITE_TEXT_ALIGNMENT> mvdwTextAlignments;  //TextAllignments

    GradientColors* pGradientColors;

    ComPtr<ID3D11DeviceContext> mpd3d11DeviceContext;
    ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;
    ComPtr<ID3D11On12Device> mpd3d11On12Device;
    
    ComPtr<ID2D1Factory3> mpd2dFactory;
    ComPtr<ID2D1Device2> mpd2dDevice;
    ComPtr<ID3D11Device> pd3d11Device;
    ComPtr<IDXGIDevice> pdxgiDevice;

    ComPtr<IDWriteFactory5> mpd2dWriteFactory;
    ComPtr<IDWriteFontFile> mdwFontFile;
    ComPtr<IDWriteFontSetBuilder1> mdwFontSetBuilder;
    ComPtr<IDWriteInMemoryFontFileLoader> mdwInMemoryFontFileLoader;
    ComPtr<IDWriteFontSet> mdwFontSet;
    ComPtr<IDWriteFontFamily> mdwFontFamily;
    ComPtr<IDWriteLocalizedStrings> mdwLocalizedStrings;
    
    ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;

    std::vector<ComPtr<IDWriteFontCollection1>> mdwFontCollection;
    std::vector<ComPtr<IDWriteTextFormat>> mvdwTextFormat;
    std::vector<ComPtr<ID2D1SolidColorBrush>> mvd2dSolidBrush;
    std::vector<ComPtr<ID3D11Resource>>    mvWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      mvd2dRenderTargets;

    TCHAR mtcFontName[50];

    IWICImagingFactory* mWICFactoryPtr;
};