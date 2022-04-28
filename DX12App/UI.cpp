#include "stdafx.h"
#include "UI.h"
UI::UI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{
    UI::Initialize(device, pd3dCommandQueue);
}

UI::~UI() 
{
    for (auto& bitmap : mvBitmaps)
    {
        if(bitmap!=NULL)
            bitmap->Release();
        bitmap = NULL;
    }
}

void UI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D12CommandQueue> ppd3dCommandQueues[] = { pd3dCommandQueue };
    ThrowIfFailed(::D3D11On12CreateDevice(device.Get(), d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues),
        _countof(ppd3dCommandQueues), 0, &pd3d11Device, &mpd3d11DeviceContext, nullptr));
    ThrowIfFailed(pd3d11Device.As(&mpd3d11On12Device));
    
#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* pd3dInfoQueue;
    if (SUCCEEDED(device.Get()->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
    {
        D3D12_MESSAGE_SEVERITY pd3dSeverities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        D3D12_MESSAGE_ID pd3dDenyIds[] =
        {
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
        };

        D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
        d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
        d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
        d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
        d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

        pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
    }
    pd3dInfoQueue->Release();
#endif
    ThrowIfFailed(mpd3d11On12Device.As(&pdxgiDevice));
    ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &mpd2dFactory));
    ThrowIfFailed(mpd2dFactory->CreateDevice(pdxgiDevice.Get(), &mpd2dDevice));
    ThrowIfFailed(mpd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mpd2dDeviceContext));

    mpd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), (IUnknown**)&mpd2dWriteFactory));

    //mvBitmaps.resize(2);

}

void UI::SetVectorSize(UINT nFrame)
{
    mvWrappedRenderTargets.resize(nFrame);
    //mvdwTextFormat.resize(TextCnt);
    mvd2dRenderTargets.resize(nFrame);
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
    
    mpd3d11On12Device->AcquireWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), 1);
    mpd2dDeviceContext->SetTarget(mvd2dRenderTargets[nFrame].Get());
    mpd2dDeviceContext->BeginDraw();
}

void UI::TextDraw(const std::vector<TextBlock> &mvTextBlocks)
{
    std::wstring Text;

    //0번은 테두리 색.
    for (int i =0; i < static_cast<int>(miTextCnt); ++i)
    {
        Text.assign(mvTextBlocks[i].strText.begin(), mvTextBlocks[i].strText.end());
        mpd2dDeviceContext->DrawTextW(Text.c_str(), static_cast<UINT>(mvTextBlocks[i].strText.length()),
            mvdwTextFormat[i].Get(), mvTextBlocks[i].d2dLayoutRect, mvd2dSolidBrush[static_cast<size_t>(i)].Get());
    }
}

HRESULT UI::LoadBitmapResourceFromFile(std::wstring ImageName, int index)
{
    ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mWICFactoryPtr)));
    ComPtr<IWICBitmapDecoder> pDecoder;     
    ComPtr<IWICBitmapFrameDecode> pFrame;   
    ComPtr<IWICFormatConverter> pConverter;

    ComPtr<IWICStream> pStream;
    ComPtr<IWICBitmapScaler> pScaler;

    HRESULT hresult = mWICFactoryPtr->CreateDecoderFromFilename(ImageName.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
    if (SUCCEEDED(hresult)) hresult = pDecoder->GetFrame(0, &pFrame);
    if (SUCCEEDED(hresult)) hresult = mWICFactoryPtr->CreateFormatConverter(&pConverter);
    if (SUCCEEDED(hresult)) hresult = pConverter->Initialize(pFrame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (SUCCEEDED(hresult)) hresult = mpd2dDeviceContext->CreateBitmapFromWicBitmap(pConverter.Get(), NULL, &mvBitmaps[index]);
    
    return hresult;
}

void UI::FontLoad(const std::vector<std::wstring>& FontFilePaths)
{
    int i = 0;
    mdwFontCollection.resize(FontFilePaths.size());
    for (auto& FontFilePath : FontFilePaths)
    {
        mpd2dWriteFactory->CreateFontFileReference(FontFilePath.c_str(), nullptr, &mdwFontFile);
        mpd2dWriteFactory->CreateFontSetBuilder(&mdwFontSetBuilder);

        //mpd2dWriteFactory->CreateInMemoryFontFileLoader(&mIDWriteInMemoryFontFileLoader);
        //mpd2dWriteFactory->RegisterFontFileLoader(mIDWriteInMemoryFontFileLoader.Get());
        //mIDWriteInMemoryFontFileLoader->CreateInMemoryFontFileReference(mpd2dWriteFactory.Get(), mIDWriteFontFile.Get().)

        mdwFontSetBuilder->AddFontFile(mdwFontFile.Get());
        mdwFontSetBuilder->CreateFontSet(&mdwFontSet);

        mdwFontSetBuilder->AddFontSet(mdwFontSet.Get());

        mpd2dWriteFactory->CreateFontCollectionFromFontSet(mdwFontSet.Get(), &mdwFontCollection[i++]);
    }
    //mIDWriteFontCollection[i-1]->GetFontFamily(i-1, &mIDWriteFontFamily);
    //mIDWriteFontFamily->GetFamilyNames(&mIDWriteLocalizedStrings);
    //mIDWriteLocalizedStrings->GetString(i-1, cFontName, 50);
    

}

void UI::DrawBmp(XMFLOAT4 RectLTRB[], UINT StartNum, UINT BmpNum, const float aOpacities[])
{
    for (int i = static_cast<int>(StartNum); i < static_cast<int>(StartNum + BmpNum); ++i)
        mpd2dDeviceContext->DrawBitmap(mvBitmaps[i], D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), aOpacities[i], D2D1_INTERPOLATION_MODE_LINEAR);
}

void UI::DrawBmp(const std::vector<XMFLOAT4>& RectLTRB, UINT StartNum, UINT BmpNum, const float aOpacities[])
{
    for (int i = static_cast<int>(StartNum); i < static_cast<int>(StartNum + BmpNum); ++i)
        mpd2dDeviceContext->DrawBitmap(mvBitmaps[i], D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), aOpacities[i], D2D1_INTERPOLATION_MODE_LINEAR);
}

void UI::RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT GradientCnt, bool IsOutlined[])
{
    if (md2dLinearGradientBrush.Get())
    {
        for (size_t i = 0; i < static_cast<size_t>(GradientCnt); ++i)
        {
            if (IsOutlined[i])
                mpd2dDeviceContext->DrawRectangle(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), md2dLinearGradientBrush.Get());
            mpd2dDeviceContext->FillRectangle(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), md2dLinearGradientBrush.Get());
        }
    }
    for (size_t i = GradientCnt; i < mvd2dSolidBrush.size() - static_cast<size_t>(miTextCnt); ++i)
    {
        if (IsOutlined[i])
            mpd2dDeviceContext->DrawRectangle(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), mvd2dSolidBrush[static_cast<size_t>(i) + static_cast<size_t>(miTextCnt)].Get());
        mpd2dDeviceContext->FillRectangle(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), mvd2dSolidBrush[static_cast<size_t>(i) + static_cast<size_t>(miTextCnt)].Get());
    }
}

void UI::RoundedRectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[],  UINT GradientCnt, bool IsOutlined[])
{
    if (md2dLinearGradientBrush.Get())
    {
        for (size_t i = 0; i < static_cast<size_t>(GradientCnt); ++i)
        {
            if (IsOutlined[i])
                mpd2dDeviceContext->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), 20.0f, 20.0f), md2dLinearGradientBrush.Get());
            mpd2dDeviceContext->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), 20.0f, 20.0f), md2dLinearGradientBrush.Get());
        }
    }
    for (size_t i = GradientCnt ; i < mvd2dSolidBrush.size() - static_cast<size_t>(miTextCnt); ++i)
    {
        if (IsOutlined[i])
            mpd2dDeviceContext->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), 20.0f, 20.0f), mvd2dSolidBrush[static_cast<size_t>(i) + static_cast<size_t>(miTextCnt)].Get());
        mpd2dDeviceContext->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), 20.0f, 20.0f), mvd2dSolidBrush[static_cast<size_t>(i)+ static_cast<size_t>(miTextCnt)].Get());
    }
}

void UI::EndDraw(UINT nFrame)
{
    HRESULT hr = mpd2dDeviceContext->EndDraw();
    mpd3d11On12Device->ReleaseWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), 1);
}

void UI::Flush()
{
    if(mpd3d11DeviceContext)
        mpd3d11DeviceContext->Flush();
}

void UI::Draw(UINT nFrame/*, UINT TextCnt, UINT GradientCnt, const std::vector<TextBlock> &mvTextBlocks,
    XMFLOAT4 RectLTRB[],  XMFLOAT4 FillLTRB[]*/)
{
    
}

void UI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height)
{
    SetFrame(static_cast<float>(width), static_cast<float>(height));

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), GetFrameWidth(), GetFrameHeight());
    
    ComPtr<IDXGISurface> pdxgiSurface;
    for (UINT i = 0; i < GetRenderTargetsCount(); ++i)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        mpd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&mvWrappedRenderTargets[i]));
        mvWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        mpd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface.Get(), &d2dBitmapProperties, &mvd2dRenderTargets[i]);
    }
}

void UI::CreateFontFormat(float FontSize, const std::vector<std::wstring> &Fonts, const std::vector<DWRITE_TEXT_ALIGNMENT>& Alignment)
{
    mvdwTextFormat.resize(miTextCnt);

    for (int i = 0; i < static_cast<int>(miTextCnt); ++i)
        ThrowIfFailed(mpd2dWriteFactory->CreateTextFormat(Fonts[i].c_str(), mdwFontCollection[i].Get(), DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize, L"en-us", mvdwTextFormat[i].GetAddressOf()));
    for (int i = 0; i < static_cast<int>(miTextCnt); ++i)
    {
        ThrowIfFailed(mvdwTextFormat[i]->SetTextAlignment(Alignment[i]));
        ThrowIfFailed(mvdwTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    }
}

void UI::CreateFontFormat(std::vector<float>& FontSize, const std::vector<std::wstring>& Fonts, const std::vector<DWRITE_TEXT_ALIGNMENT>& Alignment)
{
    mvdwTextFormat.resize(miTextCnt);

    for (int i = 0; i < static_cast<int>(miTextCnt); ++i)
        ThrowIfFailed(mpd2dWriteFactory->CreateTextFormat(Fonts[i].c_str(), mdwFontCollection[i].Get(), DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize[i], L"en-us", mvdwTextFormat[i].GetAddressOf()));
    for (int i = 0; i < static_cast<int>(miTextCnt); ++i)
    {
        ThrowIfFailed(mvdwTextFormat[i]->SetTextAlignment(Alignment[i]));
        ThrowIfFailed(mvdwTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
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
    mvd2dSolidBrush.resize(static_cast<size_t>(miTextCnt)+static_cast<size_t>(miUICnt));
    //ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[0]));
    for (UINT i = 0; i < static_cast<UINT>(miTextCnt)+static_cast<UINT>(miUICnt); ++i)
        ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[i]));
    // 0번 SolidBrush는 무조건 Black, 나머지는 인자로 받은 ColorList로 설정. 따라서 Resize할 때 UI와 Text 수에다가 1을 더해서 설정
    // 1번부터는 ColorList색. Text색 이후 UI 색으로 설정
}

void UI::BuildSolidBrush(const std::vector<D2D1::ColorF>& ColorList)
{
    mvd2dSolidBrush.resize(static_cast<size_t>(miTextCnt) + static_cast<size_t>(miUICnt));
    //ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[0]));
    for (UINT i = 0; i < static_cast<UINT>(miTextCnt) + static_cast<UINT>(miUICnt); ++i)
        ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[i]));
}

void UI::BuildLinearGradientBrush(UINT ColorCnt, D2D1::ColorF* ColorList)
{
    ID2D1GradientStopCollection* pGradientStops = NULL;
    D2D1_GRADIENT_STOP* gradientStops = new D2D1_GRADIENT_STOP[ColorCnt];
    
    for (int i = 0; i < static_cast<int>(ColorCnt); ++i)
    {
        gradientStops[i].color = ColorList[i];
        gradientStops[i].position = static_cast<float>(i) * 1.0f / 3.0f;
    }
    ThrowIfFailed(mpd2dDeviceContext->CreateGradientStopCollection(gradientStops, ColorCnt, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &pGradientStops));
    ThrowIfFailed(mpd2dDeviceContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f)), D2D1::Point2F(mfWidth * (1.0f / 2.0f), mfHeight * (8.0f / 9.0f))), pGradientStops, &md2dLinearGradientBrush));

    delete[] gradientStops;
    pGradientStops->Release();
}
void UI::BuildLinearGradientBrushes(UINT GradientCnt, GradientColors GColors[])
{
    for (UINT i = 0; i < GradientCnt; ++i)
        BuildLinearGradientBrush(GColors[i].ColorCnt, GColors[i].ColorList);
}
void UI::Reset() 
{
    md2dLinearGradientBrush.Reset();
    mpd3d11DeviceContext.Reset();
    mpd3d11On12Device.Reset();
    mpd2dFactory.Reset();
    mpd2dDevice.Reset();
    pd3d11Device.Reset();
    pdxgiDevice.Reset();
    mpd2dDeviceContext.Reset();

    mpd2dWriteFactory.Reset();
    if(mWICFactoryPtr)
     mWICFactoryPtr->Release();

    mdwFontFile.Reset();
    mdwFontSetBuilder.Reset();
    mdwInMemoryFontFileLoader.Reset();
    mdwFontSet.Reset();
    //mdwFontFamily.Reset();
    //mdwLocalizedStrings.Reset();

    for (auto& TextBlock : mvTextBlocks)
        TextBlock.strText.clear();
    
    mvd2dSolidBrush.clear();

    for (auto& TextFormat : mvdwTextFormat)
        TextFormat.Reset();
    mvdwTextFormat.clear();

    for (auto &renderTarget : mvWrappedRenderTargets)
        renderTarget.Reset();
    for (auto &bitmap : mvd2dRenderTargets)
        bitmap.Reset();
    for (auto &bitmap : mvBitmaps)
        bitmap->Release();
    for (auto& FontCollection : mdwFontCollection)
        FontCollection.Reset();
    mdwFontCollection.clear();
    
    mvTextBlocks.clear();
    mvWrappedRenderTargets.clear();
    mvd2dRenderTargets.clear();
    mvBitmaps.clear();
    mvBitmapFileNames.clear();
    mvColors.clear();
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
    for (int i = 0; i < static_cast<int>(mvTextBlocks.size()); ++i)
    {
        mvTextBlocks[i].d2dLayoutRect.left = TextBlocks[i].d2dLayoutRect.left;
        mvTextBlocks[i].d2dLayoutRect.right = TextBlocks[i].d2dLayoutRect.right;
        mvTextBlocks[i].d2dLayoutRect.top = TextBlocks[i].d2dLayoutRect.top;
        mvTextBlocks[i].d2dLayoutRect.bottom = TextBlocks[i].d2dLayoutRect.bottom;
        mvTextBlocks[i].pdwFormat = TextBlocks[i].pdwFormat;
        mvTextBlocks[i].strText = TextBlocks[i].strText;
    }
}