#include "stdafx.h"
#include "UI.h"
UI::UI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    UI::Initialize(device, pd3dCommandQueue);
}

UI::~UI() 
{
    Reset();
    CoUninitialize();
}

void UI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{
    UINT D3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS D2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    D2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    D3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D12CommandQueue> D3dCommandQueues[] = { pd3dCommandQueue };
    ThrowIfFailed(::D3D11On12CreateDevice(device.Get(), D3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(D3dCommandQueues),
        _countof(D3dCommandQueues), 0, &mD3d11Device, &mD3d11DeviceContext, nullptr));
    ThrowIfFailed(mD3d11Device.As(&mD3d11On12Device));
    
#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* D3dInfoQueue;
    if (SUCCEEDED(device.Get()->QueryInterface(IID_PPV_ARGS(&D3dInfoQueue))))
    {
        D3D12_MESSAGE_SEVERITY D3dSeverities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        D3D12_MESSAGE_ID D3dDenyIds[] =
        {
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
        };

        D3D12_INFO_QUEUE_FILTER D3dInforQueueFilter = { };
        D3dInforQueueFilter.DenyList.NumSeverities = _countof(D3dSeverities);
        D3dInforQueueFilter.DenyList.pSeverityList = D3dSeverities;
        D3dInforQueueFilter.DenyList.NumIDs = _countof(D3dDenyIds);
        D3dInforQueueFilter.DenyList.pIDList = D3dDenyIds;

        D3dInfoQueue->PushStorageFilter(&D3dInforQueueFilter);
    }
    D3dInfoQueue->Release();
#endif
    ThrowIfFailed(mD3d11On12Device.As(&mDxgiDevice));
    ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &D2dFactoryOptions, &mD2dFactory));
    ThrowIfFailed(mD2dFactory->CreateDevice(mDxgiDevice.Get(), &mD2dDevice));
    ThrowIfFailed(mD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mD2dDeviceContext));

    mD2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), (IUnknown**)&mD2dWriteFactory));

    //mvBitmaps.resize(2);

}

void UI::SetVectorSize(UINT nFrame)
{
    mWrappedRenderTargets.resize(nFrame);
    //mvdwTextFormat.resize(TextCnt);
    mD2dRenderTargets.resize(nFrame);
    ResizeTextBlock(GetTextCnt());
    ResizeFontSize(GetTextCnt());
    ResizeFonts(GetTextCnt());
    ResizeTextAlignment(GetTextCnt());
    ResizeBitmapNames(GetBitmapCnt());
    SetBitmapsSize(GetBitmapCnt());
   //mvd2dLinearGradientBrush.resize(TextCnt);

}

void UI::BeginDraw(UINT nFrame)
{
    
    mD3d11On12Device->AcquireWrappedResources(mWrappedRenderTargets[nFrame].GetAddressOf(), 1);
    mD2dDeviceContext->SetTarget(mD2dRenderTargets[nFrame].Get());
    mD2dDeviceContext->BeginDraw();
}

void UI::TextDraw(const std::vector<TextBlock> &mTextBlocks)
{
    std::wstring Text;

    for (int i =0; i < static_cast<int>(mTextCnt); ++i)
    {
        Text.assign(mTextBlocks[i].strText.begin(), mTextBlocks[i].strText.end());
        mD2dDeviceContext->DrawTextW(Text.c_str(), static_cast<UINT>(mTextBlocks[i].strText.length()),
            mDWriteTextFormat[i].Get(), mTextBlocks[i].d2dLayoutRect, mD2dSolidBrush[static_cast<size_t>(i)].Get());
    }
}

HRESULT UI::LoadBitmapResourceFromFile(std::wstring ImageName, int index)
{
    ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mWICFactoryPtr)));
    ComPtr<IWICBitmapDecoder> Decoder;     
    ComPtr<IWICBitmapFrameDecode> Frame;   
    ComPtr<IWICFormatConverter> Converter;

    ComPtr<IWICStream> Stream;
    ComPtr<IWICBitmapScaler> Scaler;

    HRESULT hresult = mWICFactoryPtr->CreateDecoderFromFilename(ImageName.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &Decoder);
    if (SUCCEEDED(hresult)) hresult = Decoder->GetFrame(0, &Frame);
    if (SUCCEEDED(hresult)) hresult = mWICFactoryPtr->CreateFormatConverter(&Converter);
    if (SUCCEEDED(hresult)) hresult = Converter->Initialize(Frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (SUCCEEDED(hresult)) hresult = mD2dDeviceContext->CreateBitmapFromWicBitmap(Converter.Get(), NULL, mBitmaps[index].GetAddressOf());
    
    return hresult;
}

void UI::FontLoad(const std::vector<std::wstring>& FontFilePaths)
{
    int i = 0;
    mDWriteFontCollection.resize(FontFilePaths.size());
    for (auto& FontFilePath : FontFilePaths)
    {
        mD2dWriteFactory->CreateFontFileReference(FontFilePath.c_str(), nullptr, &mDWriteFontFile);
        mD2dWriteFactory->CreateFontSetBuilder(&mDWriteFontSetBuilder);

        //mpd2dWriteFactory->CreateInMemoryFontFileLoader(&mIDWriteInMemoryFontFileLoader);
        //mpd2dWriteFactory->RegisterFontFileLoader(mIDWriteInMemoryFontFileLoader.Get());
        //mIDWriteInMemoryFontFileLoader->CreateInMemoryFontFileReference(mpd2dWriteFactory.Get(), mIDWriteFontFile.Get().)

        mDWriteFontSetBuilder->AddFontFile(mDWriteFontFile.Get());
        mDWriteFontSetBuilder->CreateFontSet(&mDWriteFontSet);

        mDWriteFontSetBuilder->AddFontSet(mDWriteFontSet.Get());

        mD2dWriteFactory->CreateFontCollectionFromFontSet(mDWriteFontSet.Get(), &mDWriteFontCollection[i++]);
    }
    //mIDWriteFontCollection[i-1]->GetFontFamily(i-1, &mIDWriteFontFamily);
    //mIDWriteFontFamily->GetFamilyNames(&mIDWriteLocalizedStrings);
    //mIDWriteLocalizedStrings->GetString(i-1, cFontName, 50);
    

}

void UI::DrawBmp(XMFLOAT4 RectLTRB[], UINT StartNum, UINT BmpNum, const float aOpacities[])
{
    for (int i = static_cast<int>(StartNum); i < static_cast<int>(StartNum + BmpNum); ++i)
        mD2dDeviceContext->DrawBitmap(mBitmaps[i].Get(), D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), aOpacities[i], D2D1_INTERPOLATION_MODE_LINEAR);
}

void UI::DrawBmp(const std::vector<XMFLOAT4>& RectLTRB, UINT StartNum, UINT BmpNum, const float aOpacities[])
{
    for (int i = static_cast<int>(StartNum); i < static_cast<int>(StartNum + BmpNum); ++i)
        mD2dDeviceContext->DrawBitmap(mBitmaps[i].Get(), D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), aOpacities[i], D2D1_INTERPOLATION_MODE_LINEAR);
}

void UI::RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT InvisibleRectCnt, UINT GradientCnt, bool IsOutlined[])
{
    if (mD2dLinearGradientBrush.Get())
    {
        for (size_t i = 0; i < static_cast<size_t>(GradientCnt); ++i)
        {
            if (IsOutlined[i])
                mD2dDeviceContext->DrawRectangle(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), mD2dLinearGradientBrush.Get());
            mD2dDeviceContext->FillRectangle(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), mD2dLinearGradientBrush.Get());
        }
    }
    for (size_t i = GradientCnt; i < mD2dSolidBrush.size() - static_cast<size_t>(mTextCnt) - static_cast<size_t>(InvisibleRectCnt); ++i)
    {
        if (IsOutlined[i])
            mD2dDeviceContext->DrawRectangle(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), mD2dSolidBrush[static_cast<size_t>(i) + static_cast<size_t>(mTextCnt)].Get());
        mD2dDeviceContext->FillRectangle(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), mD2dSolidBrush[static_cast<size_t>(i) + static_cast<size_t>(mTextCnt)].Get());
    }
}

void UI::RoundedRectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[],  UINT GradientCnt, bool IsOutlined[])
{
    if (mD2dLinearGradientBrush.Get())
    {
        for (size_t i = 0; i < static_cast<size_t>(GradientCnt); ++i)
        {
            if (IsOutlined[i])
                mD2dDeviceContext->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), 20.0f, 20.0f), mD2dLinearGradientBrush.Get());
            mD2dDeviceContext->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), 20.0f, 20.0f), mD2dLinearGradientBrush.Get());
        }
    }
    for (size_t i = GradientCnt ; i < mD2dSolidBrush.size() - static_cast<size_t>(mTextCnt); ++i)
    {
        if (IsOutlined[i])
            mD2dDeviceContext->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), 20.0f, 20.0f), mD2dSolidBrush[static_cast<size_t>(i) + static_cast<size_t>(mTextCnt)].Get());
        mD2dDeviceContext->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), 20.0f, 20.0f), mD2dSolidBrush[static_cast<size_t>(i)+ static_cast<size_t>(mTextCnt)].Get());
    }
}

void UI::EndDraw(UINT nFrame)
{
    HRESULT hr = mD2dDeviceContext->EndDraw();
    mD3d11On12Device->ReleaseWrappedResources(mWrappedRenderTargets[nFrame].GetAddressOf(), 1);
}

void UI::Flush()
{
    if(mD3d11DeviceContext)
        mD3d11DeviceContext->Flush();
}

void UI::Draw(UINT nFrame/*, UINT TextCnt, UINT GradientCnt, const std::vector<TextBlock> &mTextBlocks,
    XMFLOAT4 RectLTRB[],  XMFLOAT4 FillLTRB[]*/)
{
    
}

void UI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height)
{
    SetFrame(static_cast<float>(width), static_cast<float>(height));

    D2D1_BITMAP_PROPERTIES1 D2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), GetFrameWidth(), GetFrameHeight());
    
    ComPtr<IDXGISurface> DxgiSurface;
    for (UINT i = 0; i < GetRenderTargetsCount(); ++i)
    {
        D3D11_RESOURCE_FLAGS D3d11Flags = { D3D11_BIND_RENDER_TARGET };
        mD3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &D3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&mWrappedRenderTargets[i]));
        mWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&DxgiSurface);
        mD2dDeviceContext->CreateBitmapFromDxgiSurface(DxgiSurface.Get(), &D2dBitmapProperties, &mD2dRenderTargets[i]);
    }
}

void UI::CreateFontFormat(float FontSize, const std::vector<std::wstring> &Fonts, const std::vector<DWRITE_TEXT_ALIGNMENT>& Alignment)
{
    mDWriteTextFormat.resize(mTextCnt);

    for (int i = 0; i < static_cast<int>(mTextCnt); ++i)
        ThrowIfFailed(mD2dWriteFactory->CreateTextFormat(Fonts[i].c_str(), mDWriteFontCollection[i].Get(), DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize, L"en-us", mDWriteTextFormat[i].GetAddressOf()));
    for (int i = 0; i < static_cast<int>(mTextCnt); ++i)
    {
        ThrowIfFailed(mDWriteTextFormat[i]->SetTextAlignment(Alignment[i]));
        ThrowIfFailed(mDWriteTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    }
}

void UI::CreateFontFormat(std::vector<float>& FontSize, const std::vector<std::wstring>& Fonts, const std::vector<DWRITE_TEXT_ALIGNMENT>& Alignment)
{
    mDWriteTextFormat.resize(mTextCnt);

    for (int i = 0; i < static_cast<int>(mTextCnt); ++i)
        ThrowIfFailed(mD2dWriteFactory->CreateTextFormat(Fonts[i].c_str(), mDWriteFontCollection[i].Get(), DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize[i], L"en-us", mDWriteTextFormat[i].GetAddressOf()));
    for (int i = 0; i < static_cast<int>(mTextCnt); ++i)
    {
        ThrowIfFailed(mDWriteTextFormat[i]->SetTextAlignment(Alignment[i]));
        ThrowIfFailed(mDWriteTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    }
}

void UI::BuildBrush(std::vector<D2D1::ColorF>& ColorList,
    UINT gradientCnt, D2D1::ColorF* gradientColors)
{
    BuildSolidBrush(ColorList);
    BuildLinearGradientBrush(gradientCnt, gradientColors);
}

void UI::BuildBrush(D2D1::ColorF* ColorList, 
    UINT gradientCnt, D2D1::ColorF* gradientColors)
{
    BuildSolidBrush(ColorList);
    BuildLinearGradientBrush(gradientCnt, gradientColors);
}

void UI::BuildSolidBrush(D2D1::ColorF* ColorList)
{
    //첫번째 SolidColorBrush는 무조건 Black으로 설정한 후 테두리로 사용할 것. 그게 편할 듯.
    mD2dSolidBrush.resize(static_cast<size_t>(mTextCnt)+static_cast<size_t>(mUICnt));
    //ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[0]));
    for (UINT i = 0; i < static_cast<UINT>(mTextCnt)+static_cast<UINT>(mUICnt); ++i)
        ThrowIfFailed(mD2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mD2dSolidBrush[i]));
    // 0번 SolidBrush는 무조건 Black, 나머지는 인자로 받은 ColorList로 설정. 따라서 Resize할 때 UI와 Text 수에다가 1을 더해서 설정
    // 1번부터는 ColorList색. Text색 이후 UI 색으로 설정
}

void UI::BuildSolidBrush(const std::vector<D2D1::ColorF>& ColorList)
{
    mD2dSolidBrush.resize(static_cast<size_t>(mTextCnt) + static_cast<size_t>(mUICnt));
    //ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[0]));
    for (UINT i = 0; i < static_cast<UINT>(mTextCnt) + static_cast<UINT>(mUICnt); ++i)
        ThrowIfFailed(mD2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mD2dSolidBrush[i]));
}

void UI::BuildLinearGradientBrush(UINT ColorCnt, D2D1::ColorF* ColorList)
{
    ID2D1GradientStopCollection* D2dGradientStopCollection = NULL;
    D2D1_GRADIENT_STOP* D2dGradientStops = new D2D1_GRADIENT_STOP[ColorCnt];
    
    for (int i = 0; i < static_cast<int>(ColorCnt); ++i)
    {
        D2dGradientStops[i].color = ColorList[i];
        D2dGradientStops[i].position = static_cast<float>(i) * 1.0f / 3.0f;
    }
    ThrowIfFailed(mD2dDeviceContext->CreateGradientStopCollection(D2dGradientStops, ColorCnt, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &D2dGradientStopCollection));
    ThrowIfFailed(mD2dDeviceContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(mWidth * (3.0f / 16.0f), mHeight * (5.0f / 6.0f)), D2D1::Point2F(mWidth * (1.0f / 2.0f), mHeight * (8.0f / 9.0f))), D2dGradientStopCollection, &mD2dLinearGradientBrush));

    delete[] D2dGradientStops;
    D2dGradientStopCollection->Release();
}
void UI::BuildLinearGradientBrushes(UINT GradientCnt, GradientColors GColors[])
{
    for (UINT i = 0; i < GradientCnt; ++i)
        BuildLinearGradientBrush(GColors[i].ColorCnt, GColors[i].ColorList);
}
void UI::Reset() 
{
    mD2dLinearGradientBrush.Reset();
    mD3d11DeviceContext.Reset();
    mD3d11On12Device.Reset();
    mD2dFactory.Reset();
    mD2dDevice.Reset();
    mD3d11Device.Reset();
    mDxgiDevice.Reset();
    mD2dDeviceContext.Reset();

    mD2dWriteFactory.Reset();
    

    mDWriteFontFile.Reset();
    mDWriteFontSetBuilder.Reset();
    mDWriteInMemoryFontFileLoader.Reset();
    mDWriteFontSet.Reset();
    //mdwFontFamily.Reset();
    //mdwLocalizedStrings.Reset();

    for (auto& TextBlock : mTextBlocks)
        TextBlock.strText.clear();
    
    mD2dSolidBrush.clear();

    for (auto& TextFormat : mDWriteTextFormat)
        TextFormat.Reset();
    mDWriteTextFormat.clear();

    for (auto &renderTarget : mWrappedRenderTargets)
        renderTarget.Reset();
    for (auto &bitmap : mD2dRenderTargets)
        bitmap.Reset();
    for (auto& FontCollection : mDWriteFontCollection)
        FontCollection.Reset();

    if(mWICFactoryPtr) mWICFactoryPtr.Reset();
    mWICFactoryPtr = nullptr;

    for (auto& bitmap : mBitmaps)
        bitmap.Reset();
    mDWriteFontCollection.clear();
    
    mTextBlocks.clear();
    mWrappedRenderTargets.clear();
    mD2dRenderTargets.clear();
    mBitmaps.clear();
    mBitmapFileNames.clear();
    mColors.clear();
}

void UI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    SetVectorSize(nFrame);
    Initialize(device, pd3dCommandQueue);
    BuildObjects(ppd3dRenderTargets, width, height);
}

void UI::SetTextBlock(const std::vector<TextBlock>& TextBlocks)
{
    for (int i = 0; i < static_cast<int>(mTextBlocks.size()); ++i)
    {
        mTextBlocks[i].d2dLayoutRect.left = TextBlocks[i].d2dLayoutRect.left;
        mTextBlocks[i].d2dLayoutRect.right = TextBlocks[i].d2dLayoutRect.right;
        mTextBlocks[i].d2dLayoutRect.top = TextBlocks[i].d2dLayoutRect.top;
        mTextBlocks[i].d2dLayoutRect.bottom = TextBlocks[i].d2dLayoutRect.bottom;
        mTextBlocks[i].pdwFormat = TextBlocks[i].pdwFormat;
        mTextBlocks[i].strText = TextBlocks[i].strText;
    }
}