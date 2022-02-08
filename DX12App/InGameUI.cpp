#include "stdafx.h"
#include "InGameUI.h"

InGameUI::InGameUI(UINT nFrame, ID3D12Device* pd3dDevice,ID3D12CommandQueue* 
    pd3dCommandQueue) : UI(nFrame, pd3dDevice, pd3dCommandQueue), TextCnt(5), UICnt(3)
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
}

InGameUI::~InGameUI()
{

}

void InGameUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame, TextCnt);
    //mvWrappedRenderTargets.resize(nFrame);
    //mvdwTextFormat.resize(TextCnt);
    //mvd2dRenderTargets.resize(nFrame);
    mvTextBlocks.resize(TextCnt);
    ////mvd2dLinearGradientBrush.resize(TextCnt);
    mvd2dTextBrush.resize(TextCnt + UICnt + 1);
}

void InGameUI::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{
//    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
//    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };
//
//#if defined(_DEBUG) || defined(DBG)
//    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
//    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
//    ComPtr<ID3D12CommandQueue> ppd3dCommandQueues[] = { pd3dCommandQueue };
//    ThrowIfFailed(::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues),
//        _countof(ppd3dCommandQueues), 0, &pd3d11Device, &mpd3d11DeviceContext, nullptr));
//    //pd3d11Device.Get()->QueryInterface(__uuidof(ID3D11On12Device), (void**)&mpd3d11On12Device);
//    ThrowIfFailed(pd3d11Device.As(&mpd3d11On12Device));
//    //pd3d11Device->Release();
//
//#if defined(_DEBUG) || defined(DBG)
//    ID3D12InfoQueue* pd3dInfoQueue;
//    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
//    {
//        D3D12_MESSAGE_SEVERITY pd3dSeverities[] =
//        {
//            D3D12_MESSAGE_SEVERITY_INFO,
//        };
//
//        D3D12_MESSAGE_ID pd3dDenyIds[] =
//        {
//            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
//        };
//
//        D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
//        d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
//        d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
//        d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
//        d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;
//
//        pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
//    }
//    pd3dInfoQueue->Release();
//#endif
//    //mpd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);
//    ThrowIfFailed(mpd3d11On12Device.As(&pdxgiDevice));
//    ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &mpd2dFactory));
//    ThrowIfFailed(mpd2dFactory->CreateDevice(pdxgiDevice.Get(), &mpd2dDevice));
//    ThrowIfFailed(mpd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mpd2dDeviceContext));
//
//    mpd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
//
//    ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&mpd2dWriteFactory));
}
void InGameUI::StartPrint(std::wstring& strUIText)
{
    mvTextBlocks[TextCnt - 1].strText = strUIText;
}
void InGameUI::Update(const std::vector<std::wstring>& strUIText)
{
    for (int i = 0; i < TextCnt; ++i)
        mvTextBlocks[i].strText = strUIText[i];
    if (fDraftGage >= 1.0f)
    {
        fDraftGage = 0.0f;
        if (uItemCnt < 2)
            uItemCnt += 1;
    }
    //DraftGage Set
    SetDraftGage();
}

void InGameUI::SetDraftGage()
{
    fDraftGage += 0.001f;
}

void InGameUI::Draw(UINT nFrame)
{
    
    /*mpd2dDeviceContext.Get()->BeginDraw();
    mpd3d11On12Device->AcquireWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), nFrame);
    mpd2dDeviceContext.Get()->SetTarget(mvd2dRenderTargets[nFrame].Get());*/
   /* for (int i = 0; i < TextCnt; ++i)
    {
        mpd2dDeviceContext.Get()->DrawTextW(mvTextBlocks[i].strText.c_str(), static_cast<UINT>(mvTextBlocks[i].strText.length()),
            mvdwTextFormat[i].Get(), mvTextBlocks[i].d2dLayoutRect, mvd2dTextBrush[i].Get());
    }*/
    XMFLOAT4 LTRB[] = 
    { 
        {mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (3.0f / 16.0f) + (mfWidth * (1.0f / 2.0f) - mfWidth * (3.0f / 16.0f)) * fDraftGage, mfHeight * (8.0f / 9.0f)}, 
        {mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)},
        {mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)}
    };
    UI::Draw(nFrame, TextCnt, mvTextBlocks, LTRB);
    //Fill Draft gage
    //mpd2dDeviceContext.Get()->FillRectangle(D2D1::RectF(mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f), mfWidth * (3.0f / 16.0f) + (mfWidth * (1.0f / 2.0f) - mfWidth * (3.0f / 16.0f)) * fDraftGage, mfHeight * (8.0f / 9.0f)),
    //    md2dLinearGradientBrush.Get());//LinearBrush - Color[4] : ForestGreen, Yellow, Orange, Red 
    ////Draft gage
    //mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f), mfWidth * (1.0f / 2.0f), mfHeight * (8.0f / 9.0f)),
    //    mvd2dTextBrush[TextCnt].Get()); //BrushColor : Black

    //// Item Slot1
    //mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f), mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)),
    //    mvd2dTextBrush[TextCnt].Get());//BrushColor : Black
    //if (uItemCnt > 0)
    //    mpd2dDeviceContext.Get()->FillRectangle(D2D1::RectF(mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f), mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)),
    //        mvd2dTextBrush[TextCnt + 2].Get()); //BrusuhColor : Red

    //    // Item Slot2
    //mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f), mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)),
    //    mvd2dTextBrush[TextCnt].Get());//BrushColor : Black
    //if (uItemCnt > 1)
    //    mpd2dDeviceContext.Get()->FillRectangle(D2D1::RectF(mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f), mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)),
    //        mvd2dTextBrush[TextCnt + 3].Get());//BrushColor : Aqua

        //Input TextRect
        /*mpd2dDeviceContext.Get()->DrawRectangle(D2D1::RectF(mfWidth * (3.0f / 32.0f), mfHeight * (2.0f / 6.0f), mfWidth * (20.0f / 32.0f), mfHeight * (3.0f / 6.0f)),
            mvd2dTextBrush[TextCnt].Get());
        mpd2dDeviceContext.Get()->DrawTextW(wsInputText.c_str(), static_cast<UINT>(wsInputText.length()),
            mvdwTextFormat[0].Get(), mvTextBlocks[4].d2dLayoutRect, mvd2dTextBrush[0].Get());*/
    /*mpd2dDeviceContext.Get()->EndDraw();

    mpd3d11On12Device->ReleaseWrappedResources(mvWrappedRenderTargets[nFrame].GetAddressOf(), 0);

    Flush();*/
    //mpd3d11DeviceContext.Get()->Flush();
}

void InGameUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 15.0f;
    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
    Fonts.push_back(L"Vladimir Script º¸Åë");
    Fonts.push_back(L"¹ÙÅÁÃ¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");

    //= { L"Tahoma" , L"Vladimir Script º¸Åë" , L"¹ÙÅÁÃ¼" , L"±¼¸²Ã¼" , L"±¼¸²Ã¼" };
    UI::CreateFontFormat(fFontSize, Fonts);
    
    //All System Font
    /*mpd2dWriteFactory->CreateTextFormat(L"Tahoma", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", mvdwTextFormat[0].GetAddressOf());
    mpd2dWriteFactory->CreateTextFormat(L"Vladimir Script º¸Åë", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", mvdwTextFormat[1].GetAddressOf());
    mpd2dWriteFactory->CreateTextFormat(L"¹ÙÅÁÃ¼", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", mvdwTextFormat[2].GetAddressOf());
    mpd2dWriteFactory->CreateTextFormat(L"±¼¸²Ã¼", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", mvdwTextFormat[3].GetAddressOf());
    mpd2dWriteFactory->CreateTextFormat(L"±¼¸²Ã¼", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", mvdwTextFormat[4].GetAddressOf());*/


    //for (int i = 0; i < TextCnt; ++i)
    //{
    //    ThrowIfFailed(mvdwTextFormat[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    //    ThrowIfFailed(mvdwTextFormat[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); // DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    //}
    
}

void InGameUI::SetTextRect()
{
    /*for (auto Text : mvTextBlocks)
        Text.pdwFormat = mvdwTextFormat[0].Get();*/

    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(0.0f, 23.0f + mfHeight / 6, mfWidth / 6, 23.0f + (mfHeight / 6));
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(0.0f, 23.0f, mfWidth / 6, mfHeight / 6);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 0.0f, mfWidth, mfHeight / 6);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 5 * (mfHeight / 6), mfWidth, mfHeight);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 8, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
}

void InGameUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    /*mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);

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
    }*/
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[9] = { D2D1::ColorF::Black, D2D1::ColorF::Black, (0xE12C38, 1.0f), (0xE12C38, 1.0f), D2D1::ColorF::OrangeRed, D2D1::ColorF::Black, D2D1::ColorF::Yellow, D2D1::ColorF::Red, D2D1::ColorF::Aqua };
    D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildBrush(TextCnt + 4, colorList, 4, gradientColors);

    //D2D1::ColorF colorList[9] = { D2D1::ColorF::Black, D2D1::ColorF::Black, (0xE12C38, 1.0f), (0xE12C38, 1.0f), D2D1::ColorF::OrangeRed, D2D1::ColorF::Black, D2D1::ColorF::Yellow, D2D1::ColorF::Red, D2D1::ColorF::Aqua };
    //BuildBrush(TextCnt + 4, colorList); //SolidBrush and LinearBrush(Gradient)

    SetTextRect();
}

void InGameUI::Flush()
{
    //mpd3d11DeviceContext.Get()->Flush();
}

void InGameUI::Reset()
{
    //mpd3d11DeviceContext.Get()->Flush();

    /*md2dLinearGradientBrush.Reset();
    mpd3d11DeviceContext.Reset();
    mpd3d11On12Device.Reset();
    mpd2dFactory.Reset();
    mpd2dWriteFactory.Reset();
    mpd2dDevice.Reset();
    pd3d11Device.Reset();
    pdxgiDevice.Reset();
    mpd2dDeviceContext.Reset();*/
    UI::Reset();
    mvTextBlocks.clear();
    //mvdwTextFormat.clear();
    /*mvd2dTextBrush.clear();

    for (auto renderTarget : mvWrappedRenderTargets)
        renderTarget.Reset();
    for (auto bitmap : mvd2dRenderTargets)
        bitmap.Reset();

    mvWrappedRenderTargets.clear();
    mvd2dRenderTargets.clear();*/
}

void InGameUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}

void InGameUI::BuildBrush(UINT UI_Cnt, D2D1::ColorF* ColorList, UINT gradientCnt, D2D1::ColorF* gradientColors)
{
    BuildSolidBrush(UI_Cnt, ColorList);
    BuildLinearGradientBrush(gradientCnt, gradientColors);
}

void InGameUI::BuildSolidBrush(UINT UI_Cnt, D2D1::ColorF* ColorList)
{
    // 0: Lap, 1: Time, 2: Rank, 3: Speed, 4: Draft Gage, 5: Item Slot1, 6: Item Slot2
    //Black, Black, DarkRed, DarkRed, Black, Yellow, Red, Aqua
    UI::BuildSolidBrush(UI_Cnt, ColorList);
   /* for (int i = 0; i < UI_Cnt; ++i)
        ThrowIfFailed(mpd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(ColorList[i]), (ID2D1SolidColorBrush**)&mvd2dTextBrush[i]));*/
}

void InGameUI::BuildLinearGradientBrush(UINT gradientCnt, D2D1::ColorF* gradientColors)
{
    //ID2D1GradientStopCollection* pGradientStops = NULL;

    //D2D1_GRADIENT_STOP gradientStops[4];
    UI::BuildLinearGradientBrush(gradientCnt, gradientColors);
    /*for (int i = 0; i < 4; ++i)
    {
        gradientStops[i].color = gradientColors[i];
        gradientStops[i].position = static_cast<float>(i) * 1.0f / 3.0f;
    }

    ThrowIfFailed(mpd2dDeviceContext->CreateGradientStopCollection(gradientStops, 4, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &pGradientStops));
    ThrowIfFailed(mpd2dDeviceContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f)), D2D1::Point2F(mfWidth * (1.0f / 2.0f), mfHeight * (8.0f / 9.0f))), pGradientStops, &md2dLinearGradientBrush));

    pGradientStops->Release();*/
}