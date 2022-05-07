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
    virtual ~UI();
    virtual void Update(float GTime, Player* mPlayer) {}
    virtual void Update(float GTime, std::vector<std::string>& Texts) {} 
    virtual void Update(std::vector<std::string>& Texts) {}
    virtual void Update(float GTime) {}
    virtual void Draw(UINT nFrame);
    virtual void SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) {}

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
    virtual void ChangeTextAlignment(UINT uNum, UINT uState) {}

    void BeginDraw(UINT nFrame);
    void DrawBmp(XMFLOAT4 RectLTRB[], UINT StartNum, UINT BmpNum, const float aOpacities[]);
    void DrawBmp(const std::vector<XMFLOAT4>& RectLTRB, UINT StartNum, UINT BmpNum, const float aOpacities[]);
    void TextDraw(const std::vector<TextBlock> &mTextBlocks);
    void RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT InvisibleRectCnt, UINT GradientCnt, bool IsOutlined[]);
    void RoundedRectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT GradientCnt, bool IsOutlined[]);
    void EndDraw(UINT nFrame);
    void Flush();

    UINT GetRenderTargetsCount() const { return static_cast<UINT>(mWrappedRenderTargets.size()); }

   virtual std::pair<const std::string&, const std::string&> GetLoginPacket() { return std::make_pair("", ""); }
   virtual int GetLobbyPacket() const { return -1; }
   virtual int GetRoomPacket() const { return -1; }
   
   //Frame
   void SetFrame(float W, float H) { mHeight = H; mWidth = W; }
   std::pair<float, float> GetFrame() const { return std::make_pair(mWidth, mHeight); }
   float GetFrameWidth() const { return mWidth; }
   float GetFrameHeight() const { return mHeight; }

   //Font 
   TCHAR* GetFontName() { return mtcFontName; }
   void FontLoad(const std::vector<std::wstring>& FontFilePath);

   //TextCnt
   UINT GetTextCnt() const { return mTextCnt; }
   void SetTextCnt(UINT n) { mTextCnt = n; }

   //RoundRectCnt
   UINT GetRoundRectCnt() const { return mRoundRectCnt; }
   void SetRoundRectCnt(UINT n) { mRoundRectCnt = n; }

   // RectCnt
   UINT GetRectCnt() const { return mRectCnt; }
   void SetRectCnt(UINT n) { mRectCnt = n; }

   //Ellipse
   UINT GetEllipseCnt() const { return mEllipseCnt; }
   void SetEllipseCnt(UINT n) { mEllipseCnt  = n; }

   //Gradient
   UINT GetGradientCnt() const { return mGradientCnt; }
   void SetGradientCnt(UINT n) { mGradientCnt = n; }

   // Bitmap
   UINT GetBitmapCnt() const { return mBitmapCnt; }
   void SetBitmapCnt(UINT n) { mBitmapCnt = n; }
   void SetBitmapsSize(UINT size) { mBitmaps.resize(size); }

  //Font Size
   void ResizeFontSize(UINT n) { mFontSizes.resize(n); }
   void SetFontSize(const std::vector<float>& vfFontsizes) { for (int i = 0; i < static_cast<int>(GetTextCnt());++i) mFontSizes[i] = vfFontsizes[i]; }
   void SetFontSize(float FontSize, int index) { mFontSizes[index] = FontSize; }
   float GetFontSize(int index) const { return mFontSizes[index]; }
   std::vector<float>& GetFontSize() { return mFontSizes; }

   //Fonts
   void ResizeFonts(UINT n) { mFonts.resize(n); }
   std::vector<std::wstring>& GetFonts() { return mFonts; }
   void SetFonts(const std::vector<std::wstring>& Fonts) { for (int i = 0; i < static_cast<int>(GetTextCnt()); ++i) mFonts[i] = Fonts[i]; }

   //BitmapFileName
   void ResizeBitmapNames(UINT n) { mBitmapFileNames.resize(n); }
   std::vector<std::wstring>& GetBitmapFileNames() { return mBitmapFileNames; }
   void SetBitmapFileNames(const std::vector<std::wstring>& names) { for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i) mBitmapFileNames[i] = names[i]; }

   //TextBlock
   void ResizeTextBlock(UINT n) { mTextBlocks.resize(n); }
   std::vector<TextBlock>& GetTextBlock() { return mTextBlocks; }
   void SetTextBlock(const std::vector<TextBlock>& TextBlocks);

   //Colors - no resize
   std::vector<D2D1::ColorF>& GetColors() { return mColors; }
   void SetColors(const std::vector<D2D1::ColorF>& Colors) { for (auto& color : Colors) mColors.push_back(color); }
   void SetColors(D2D1::ColorF* Colors) { for (int i = 0; i < static_cast<int>(mColors.size()); ++i) mColors.push_back(Colors[i]); }
   void SetIndexColor(int index, D2D1::ColorF Color) { mColors[index] = Color; }

   //TextAllignments
   void ResizeTextAlignment(UINT n) { mDWriteTextAlignments.resize(n); }
   std::vector<DWRITE_TEXT_ALIGNMENT>& GetTextAlignment() { return mDWriteTextAlignments; }
   void SetTextAllignments(const std::vector< DWRITE_TEXT_ALIGNMENT>& Allignments) { for (int i = 0; i < static_cast<int>(GetTextCnt());++i) mDWriteTextAlignments[i] = Allignments[i]; }
   
   //ItemCnt
   UINT GetgItemCnt() const { return mgItemCnt; };
   void SetgItem(int item) { mgItemCnt = item; }
   //UI
   void SetUICnt() { mUICnt = mRectCnt + mRoundRectCnt + mEllipseCnt; }
private:
    float mHeight = 0.0f;
    float mWidth = 0.0f;

    //Text and UI Count
    UINT mTextCnt = 0;
    UINT mRectCnt = 0;
    UINT mRoundRectCnt = 0;
    UINT mEllipseCnt = 0;
    UINT mGradientCnt = 0;
    UINT mBitmapCnt = 0;
    UINT mUICnt = 0;

    UINT mgItemCnt = 0;
    
    std::vector<float> mFontSizes; //FontSize
    std::vector<std::wstring> mFonts; //Fonts
    std::vector<std::wstring> mBitmapFileNames; //BitmapFileNames
    std::vector<ComPtr<ID2D1Bitmap>> mBitmaps; //Bitmaps
    std::vector<TextBlock>          mTextBlocks; //TextBlocks
    std::vector<D2D1::ColorF> mColors; //Colors
    std::vector<DWRITE_TEXT_ALIGNMENT> mDWriteTextAlignments;  //TextAllignments

    GradientColors* mGradientColors;

    ComPtr<ID3D11DeviceContext> mD3d11DeviceContext;
    ComPtr<ID2D1DeviceContext2> mD2dDeviceContext;
    ComPtr<ID3D11On12Device> mD3d11On12Device;
    
    ComPtr<ID2D1Factory3> mD2dFactory;
    ComPtr<ID2D1Device2> mD2dDevice;
    ComPtr<ID3D11Device> mD3d11Device;
    ComPtr<IDXGIDevice> mDxgiDevice;

    ComPtr<IDWriteFactory5> mD2dWriteFactory;
    ComPtr<IDWriteFontFile> mDWriteFontFile;
    ComPtr<IDWriteFontSetBuilder1> mDWriteFontSetBuilder;
    ComPtr<IDWriteInMemoryFontFileLoader> mDWriteInMemoryFontFileLoader;
    ComPtr<IDWriteFontSet> mDWriteFontSet;
    ComPtr<IDWriteFontFamily> mDWriteFontFamily;
    ComPtr<IDWriteLocalizedStrings> mDWriteLocalizedStrings;
    
    ComPtr<ID2D1LinearGradientBrush> mD2dLinearGradientBrush;

    std::vector<ComPtr<IDWriteFontCollection1>> mDWriteFontCollection;
    std::vector<ComPtr<IDWriteTextFormat>> mDWriteTextFormat;
    std::vector<ComPtr<ID2D1SolidColorBrush>> mD2dSolidBrush;
    std::vector<ComPtr<ID3D11Resource>>    mWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      mD2dRenderTargets;

    TCHAR mtcFontName[50];

    ComPtr<IWICImagingFactory> mWICFactoryPtr;
};