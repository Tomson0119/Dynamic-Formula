#include "stdafx.h"
#include "UI.h"
UI::UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{
    UI::Initialize(pd3dDevice, pd3dCommandQueue);
}
UI::~UI() 
{

}
void UI::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D12CommandQueue> ppd3dCommandQueues[] = { pd3dCommandQueue };
    ThrowIfFailed(::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues),
        _countof(ppd3dCommandQueues), 0, &pd3d11Device, &mpd3d11DeviceContext, nullptr));
    //pd3d11Device.Get()->QueryInterface(__uuidof(ID3D11On12Device), (void**)&mpd3d11On12Device);
    ThrowIfFailed(pd3d11Device.As(&mpd3d11On12Device));
    //pd3d11Device->Release();

#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* pd3dInfoQueue;
    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
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
    //mpd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);
    ThrowIfFailed(mpd3d11On12Device.As(&pdxgiDevice));
    ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &mpd2dFactory));
    ThrowIfFailed(mpd2dFactory->CreateDevice(pdxgiDevice.Get(), &mpd2dDevice));
    ThrowIfFailed(mpd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mpd2dDeviceContext));

    mpd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&mpd2dWriteFactory));
}

void UI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    mvWrappedRenderTargets.resize(nFrame);
    mvdwTextFormat.resize(TextCnt);
    mvd2dRenderTargets.resize(nFrame);
    //mvTextBlocks.resize(TextCnt);
    //mvd2dLinearGradientBrush.resize(TextCnt);
}
void UI::BeginDraw(UINT nFrame)
{
    mpd2dDeviceContext.Get()->BeginDraw();
    mpd3d11On12Device->AcquireWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), nFrame);
    mpd2dDeviceContext.Get()->SetTarget(mvd2dRenderTargets[nFrame].Get());
}

void UI::TextDraw(UINT nFrame, UINT TextCnt, std::vector<TextBlock> mvTextBlocks)
{
    for (int i = 0; i < TextCnt; ++i)
    {
        mpd2dDeviceContext.Get()->DrawTextW(mvTextBlocks[i].strText.c_str(), static_cast<UINT>(mvTextBlocks[i].strText.length()),
            mvdwTextFormat[i].Get(), mvTextBlocks[i].d2dLayoutRect, mvd2dTextBrush[i].Get());
    }
}
void UI::RectDraw(XMFLOAT4 LTRB[], UINT BrushCnt, std::vector<ID2D1Brush*> BrushList)
{
    for (int i = 0; i < BrushCnt; ++i)
    {
        mpd2dDeviceContext.Get()->FillRectangle(D2D1::RectF(LTRB[i].x, LTRB[i].y, LTRB[i].z, LTRB[i].w), BrushList[i]);
        mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(LTRB[i].x, LTRB[i].y, LTRB[i].z, LTRB[i].w), BrushList[i]);
    }
}

void UI::EndDraw(UINT nFrame)
{
    mpd2dDeviceContext.Get()->EndDraw();

    mpd3d11On12Device->ReleaseWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), 0);

    Flush();
}
void UI::Flush()
{
    mpd3d11DeviceContext.Get()->Flush();
}
void UI::Update(const std::vector<std::wstring>& strUIText)
{

}

void UI::Draw(UINT nFrame, UINT TextCnt, std::vector<TextBlock> mvTextBlocks,
    XMFLOAT4 LTRB[], UINT BrushCnt, std::vector<ID2D1Brush*> BrushList)
{
    BeginDraw(nFrame);
    TextDraw(nFrame, TextCnt, mvTextBlocks);
    RectDraw(LTRB, BrushCnt, BrushList);
    EndDraw(nFrame);
}

void UI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height)
{
    mfWidth = static_cast<float>(width);
    mfHeight = static_cast<float>(height);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), mfWidth, mfHeight);
    ComPtr<IDXGISurface> pdxgiSurface;
    for (UINT i = 0; i < GetRenderTargetsCount(); ++i)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        mpd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET, IID_PPV_ARGS(&mvWrappedRenderTargets[i]));
        mvWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        mpd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface.Get(), &d2dBitmapProperties, &mvd2dRenderTargets[i]);
    }
    //float fFontSize = mfHeight / 15.0f;
    //std::vector<std::wstring> Fonts = { L"Tahoma" , L"Vladimir Script ����" , L"����ü" , L"����ü" , L"����ü" };
    //CreateFontFormat(fFontSize, Fonts, mvdwTextFormat);
    //CreateFontFormat();

    //D2D1::ColorF colorList[9] = { D2D1::ColorF::Black, D2D1::ColorF::Black, (0xE12C38, 1.0f), (0xE12C38, 1.0f), D2D1::ColorF::OrangeRed, D2D1::ColorF::Black, D2D1::ColorF::Yellow, D2D1::ColorF::Red, D2D1::ColorF::Aqua };
    //BuildBrush(TextCnt + 4, colorList); //SolidBrush and LinearBrush(Gradient)

    //SetTextRect();
}
void UI::CreateFontFormat(float FontSize, std::vector<std::wstring> Fonts)
{
    //ComPtr<IDWriteTextFormat> TestTextFormat;
    for (int i = 0; i < Fonts.size(); ++i)
        ThrowIfFailed(mpd2dWriteFactory->CreateTextFormat(Fonts[i].c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize, L"en-us", mvdwTextFormat[i].GetAddressOf()));

    for (int i = 0; i < Fonts.size(); ++i)
    {
        ThrowIfFailed(mvdwTextFormat[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        ThrowIfFailed(mvdwTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    }
}

void UI::BuildBrush(UINT UI_Cnt, D2D1::ColorF* ColorList, UINT gradientCnt, D2D1::ColorF* gradientColors)
{
    BuildSolidBrush(UI_Cnt, ColorList);
    BuildLinearGradientBrush(gradientCnt, gradientColors);
}

void UI::BuildSolidBrush(UINT UI_Cnt, D2D1::ColorF* ColorList) 
{
    mvd2dTextBrush.resize(UI_Cnt+1);
    for (int i = 0; i < UI_Cnt; ++i)
        ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mvd2dTextBrush[i]));
}

void UI::BuildLinearGradientBrush(UINT ColorCnt, D2D1::ColorF ColorList[]) 
{
    ID2D1GradientStopCollection* pGradientStops = NULL;
    D2D1_GRADIENT_STOP* gradientStops = new D2D1_GRADIENT_STOP[ColorCnt];
    
    for (int i = 0; i < ColorCnt; ++i)
    {
        gradientStops[i].color = ColorList[i];
        gradientStops[i].position = static_cast<float>(i) * 1.0f / 3.0f;
    }
    ThrowIfFailed(mpd2dDeviceContext->CreateGradientStopCollection(gradientStops, 4, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &pGradientStops));
    ThrowIfFailed(mpd2dDeviceContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f)), D2D1::Point2F(mfWidth * (1.0f / 2.0f), mfHeight * (8.0f / 9.0f))), pGradientStops, &md2dLinearGradientBrush));

    delete[] gradientStops;
    pGradientStops->Release();
}

void UI::Reset() 
{
    md2dLinearGradientBrush.Reset();
    mpd3d11DeviceContext.Reset();
    mpd3d11On12Device.Reset();
    mpd2dFactory.Reset();
    mpd2dWriteFactory.Reset();
    mpd2dDevice.Reset();
    pd3d11Device.Reset();
    pdxgiDevice.Reset();
    mpd2dDeviceContext.Reset();

    mvd2dTextBrush.clear();
    mvdwTextFormat.clear();

    for (auto renderTarget : mvWrappedRenderTargets)
        renderTarget.Reset();
    for (auto bitmap : mvd2dRenderTargets)
        bitmap.Reset();

    mvWrappedRenderTargets.clear();
    mvd2dRenderTargets.clear();
}
void UI::OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height, UINT TextCnt)
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}