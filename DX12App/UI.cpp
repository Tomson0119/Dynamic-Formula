#include "stdafx.h"
#include "UI.h"

UI::UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{
    m_fWidth = 0.0f;
    m_fHeight = 0.0f;

    m_vWrappedRenderTargets.resize(nFrame);
    m_vd2dRenderTargets.resize(nFrame);
    m_vTextBlocks.resize(4);
    Initialize(pd3dDevice, pd3dCommandQueue);
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
    ::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), 
        _countof(ppd3dCommandQueues), 0, &pd3d11Device, &m_pd3d11DeviceContext, nullptr);
    pd3d11Device.Get()->QueryInterface(__uuidof(ID3D11On12Device), (void**)&m_pd3d11On12Device);
    //ThrowIfFailed(pd3d11Device.As(&m_pd3d11On12Device));
    pd3d11Device->Release();

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
    m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);
    //ThrowIfFailed(m_pd3d11On12Device.As(&pdxgiDevice));
   
    ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_pd2dFactory));
    ThrowIfFailed(m_pd2dFactory->CreateDevice(pdxgiDevice.Get(), &m_pd2dDevice));
    ThrowIfFailed(m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext));

    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    ThrowIfFailed(m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), (ID2D1SolidColorBrush**)&m_pd2dTextBrush));

    ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pd2dWriteFactory));
}

void UI::UpdateLabels(const std::vector<std::wstring>& strUIText)
{
    m_vTextBlocks[0] = { strUIText[0], D2D1::RectF(0.0f,  23.0f + m_fHeight / 6, m_fWidth / 6, 23.0f + (m_fHeight / 6)), m_pdwTextFormat.Get() };
    m_vTextBlocks[1] = { strUIText[1], D2D1::RectF(0.0f, 23.0f, m_fWidth/6, m_fHeight/6), m_pdwTextFormat.Get() };
    m_vTextBlocks[2] = { strUIText[2], D2D1::RectF(5*(m_fWidth/6), 0.0f, m_fWidth, m_fHeight / 6), m_pdwTextFormat.Get() };
    m_vTextBlocks[3] = { strUIText[3], D2D1::RectF(5 * (m_fWidth / 6), 5*(m_fHeight/6), m_fWidth, m_fHeight), m_pdwTextFormat.Get() };
}

void UI::Draw(UINT nFrame)
{
    //ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame].Get() };

    m_pd2dDeviceContext.Get()->BeginDraw();
    //m_pd3d11On12Device->AcquireWrappedResources(m_vWrappedRenderTargets[nFrame].GetAddressOf(), nFrame);
    //m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    m_pd2dDeviceContext.Get()->SetTarget(m_vd2dRenderTargets[nFrame].Get());
     
    for (auto textBlock : m_vTextBlocks)
    {
        //function blabla(L"Font");
        m_pd2dDeviceContext.Get()->DrawTextW(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), 
            textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush.Get());
    }
    m_pd2dDeviceContext.Get()->EndDraw();
   
    m_pd3d11On12Device->ReleaseWrappedResources(m_vWrappedRenderTargets[nFrame].GetAddressOf(), 0);

    m_pd3d11DeviceContext.Get()->Flush();

}

void UI::ReleaseResources()
{
    

}

void UI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
   

    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, 
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), m_fWidth, m_fHeight); 
    ComPtr<IDXGISurface> pdxgiSurface;
    for (UINT i = 0; i < GetRenderTargetsCount(); ++i)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, 
            D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface.Get(), &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        //ThrowIfFailed(pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pd3dCommandAllocator)));
    }
    //const float fSmallFontSize = m_fHeight / 40.0f;
    const float fFontSize = m_fHeight / 25.0f;

    m_pd2dWriteFactory->CreateTextFormat(L"¹ÙÅÁÃ¼", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);

    ThrowIfFailed(m_pdwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    ThrowIfFailed(m_pdwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
}