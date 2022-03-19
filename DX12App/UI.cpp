#include "stdafx.h"
#include "UI.h"
UI::UI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{
    UI::Initialize(device, pd3dCommandQueue);
}

UI::~UI() 
{

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

    ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&mpd2dWriteFactory));
}

void UI::SetVectorSize(UINT nFrame)
{
    mvWrappedRenderTargets.resize(nFrame);
    //mvdwTextFormat.resize(TextCnt);
    mvd2dRenderTargets.resize(nFrame);
   //mvd2dLinearGradientBrush.resize(TextCnt);
}

void UI::BeginDraw(UINT nFrame)
{
    mpd3d11On12Device->AcquireWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), 1);
    mpd2dDeviceContext.Get()->SetTarget(mvd2dRenderTargets[nFrame].Get());
    mpd2dDeviceContext.Get()->BeginDraw();
}

void UI::TextDraw(UINT nFrame, UINT TextCnt, const std::vector<TextBlock> &mvTextBlocks)
{
    //0번은 테두리 색.
    for (int i =0; i < static_cast<int>(TextCnt); ++i)
    {
        mpd2dDeviceContext.Get()->DrawTextW(mvTextBlocks[i].strText.c_str(), static_cast<UINT>(mvTextBlocks[i].strText.length()),
            mvdwTextFormat[i].Get(), mvTextBlocks[i].d2dLayoutRect, mvd2dSolidBrush[i+1].Get());
    }
}

void UI::RectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT TextCnt, UINT noFill, UINT GradientCnt)
{
    if (md2dLinearGradientBrush.Get())
    {
        for (int i = 0; i < static_cast<int>(GradientCnt); ++i)
        {
            mpd2dDeviceContext.Get()->FillRectangle(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), md2dLinearGradientBrush.Get());
            mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), mvd2dSolidBrush[0].Get());
        }
    }
    for (int i = GradientCnt; i < mvd2dSolidBrush.size() - TextCnt; ++i)
            mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), mvd2dSolidBrush[0].Get());
    
    for (int i = GradientCnt; i < mvd2dSolidBrush.size() - TextCnt-noFill; ++i)
            mpd2dDeviceContext.Get()->FillRectangle(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), mvd2dSolidBrush[i+TextCnt].Get());
    
}

void UI::RoundedRectDraw(XMFLOAT4 RectLTRB[], XMFLOAT4 FillLTRB[], UINT TextCnt, UINT bias, UINT GradientCnt)
{
    if (md2dLinearGradientBrush.Get())
    {
        for (int i = 0; i < static_cast<int>(GradientCnt); ++i)
        {
            mpd2dDeviceContext.Get()->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), 10.0f, 10.0f), md2dLinearGradientBrush.Get());
            mpd2dDeviceContext.Get()->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), 10.0f, 10.0f), mvd2dSolidBrush[0].Get());
        }
    }
    for (int i = GradientCnt; i < mvd2dSolidBrush.size() - TextCnt; ++i)
        mpd2dDeviceContext.Get()->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(RectLTRB[i].x, RectLTRB[i].y, RectLTRB[i].z, RectLTRB[i].w), 10.0f, 10.0f), mvd2dSolidBrush[0].Get());

    for (int i = GradientCnt; i < mvd2dSolidBrush.size() - TextCnt - bias; ++i)
        mpd2dDeviceContext.Get()->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(FillLTRB[i].x, FillLTRB[i].y, FillLTRB[i].z, FillLTRB[i].w), 10.0f, 10.0f), mvd2dSolidBrush[i + TextCnt].Get());

}

void UI::EndDraw(UINT nFrame)
{
    mpd2dDeviceContext.Get()->EndDraw();
    mpd3d11On12Device->ReleaseWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), 1);
}

void UI::Flush()
{
    mpd3d11DeviceContext.Get()->Flush();
}

void UI::Update(float GTime)
{

}

void UI::Draw(UINT nFrame/*, UINT TextCnt, UINT GradientCnt, const std::vector<TextBlock> &mvTextBlocks,
    XMFLOAT4 RectLTRB[],  XMFLOAT4 FillLTRB[]*/)
{
    
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
        mpd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&mvWrappedRenderTargets[i]));
        mvWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        mpd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface.Get(), &d2dBitmapProperties, &mvd2dRenderTargets[i]);
    }
}

void UI::CreateFontFormat(float FontSize, const std::vector<std::wstring> &Fonts, UINT TextCnt, DWRITE_TEXT_ALIGNMENT* Alignment)
{
    mvdwTextFormat.resize(TextCnt);

    for (int i = 0; i < static_cast<int>(TextCnt); ++i)
        ThrowIfFailed(mpd2dWriteFactory->CreateTextFormat(Fonts[i].c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize, L"en-us", mvdwTextFormat[i].GetAddressOf()));
    for (int i = 0; i < static_cast<int>(TextCnt); ++i)
    {
        ThrowIfFailed(mvdwTextFormat[i]->SetTextAlignment(Alignment[i]));
        ThrowIfFailed(mvdwTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    }
}

void UI::BuildBrush(UINT UICnt, UINT TextCnt, D2D1::ColorF* ColorList, 
    UINT gradientCnt, D2D1::ColorF* gradientColors)
{
    BuildSolidBrush(UICnt, TextCnt, ColorList);
    BuildLinearGradientBrush(gradientCnt, gradientColors);
}

void UI::BuildSolidBrush(UINT UICnt, UINT TextCnt, D2D1::ColorF* ColorList)
{
    //첫번째 SolidColorBrush는 무조건 Black으로 설정한 후 테두리로 사용할 것. 그게 편할 듯.
    mvd2dSolidBrush.resize(TextCnt+UICnt);
    ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[0]));
    for (int i = 0; i < static_cast<int>(TextCnt+UICnt-1); ++i)
        ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[i+1]));
    // 0번 SolidBrush는 무조건 Black, 나머지는 인자로 받은 ColorList로 설정. 따라서 Resize할 때 UI와 Text 수에다가 1을 더해서 설정
    // 1번부터는 ColorList색. Text색 이후 UI 색으로 설정
}

void UI::BuildSolidBrush(UINT UICnt, UINT TextCnt, std::vector<D2D1::ColorF>& ColorList)
{
    mvd2dSolidBrush.resize(TextCnt + UICnt);
    ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[0]));
    for (int i = 0; i < static_cast<int>(TextCnt + UICnt - 1); ++i)
        ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mvd2dSolidBrush[i + 1]));
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

    mvd2dSolidBrush.clear();
    mvdwTextFormat.clear();

    for (auto renderTarget : mvWrappedRenderTargets)
        renderTarget.Reset();
    for (auto bitmap : mvd2dRenderTargets)
        bitmap.Reset();

    mvWrappedRenderTargets.clear();
    mvd2dRenderTargets.clear();
}

void UI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    SetVectorSize(nFrame);
    Initialize(device, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}
