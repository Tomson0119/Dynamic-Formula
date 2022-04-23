#include "stdafx.h"
#include "RoomUI.h"
RoomUI::RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    // Text: 11, RoundRect: 13
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI: NicknameBox[8], StartBox, CarSelectBox[2], MapSelectBox[2]
{
    SetTextCnt(11);
    SetRoundRectCnt(13);
    SetBitmapCnt(2);
    SetUICnt();

    SetVectorSize(nFrame);
    Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

RoomUI::~RoomUI()
{

}

void RoomUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

}

void RoomUI::SetVectorSize(UINT nFrame)
{
    UI::SetVectorSize(nFrame);
  
    std::vector<std::wstring> BitmapFileNames;
    BitmapFileNames.push_back(L"Resources\\SampleImg.jpg");
    BitmapFileNames.push_back(L"Resources\\SampleImg.jpg");

    SetBitmapFileNames(BitmapFileNames);
    //mvd2dLinearGradientBrush.resize(TextCnt);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Fonts\\Blazed.ttf");
    Fonts.push_back(L"Fonts\\Xenogears.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");


    FontLoad(Fonts);
}

void RoomUI::Update(float GTime)
{
    for (int i = 0; i < 11; ++i)
        GetTextBlock()[i].strText.clear();
    for (auto& wc : std::wstring{ L"StartOrReady" })
        GetTextBlock()[0].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"CarSelect" })
        GetTextBlock()[1].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"MapSelect" })
        GetTextBlock()[2].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname1" })
        GetTextBlock()[3].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname2" })
        GetTextBlock()[4].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname3" })
        GetTextBlock()[5].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname4" })
        GetTextBlock()[6].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname5" })
        GetTextBlock()[7].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname6" })
        GetTextBlock()[8].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname7" })
        GetTextBlock()[9].strText.push_back(wc);
    for (auto& wc : std::wstring{ L"Nickname8" })
        GetTextBlock()[10].strText.push_back(wc);
    
}

void RoomUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {
        GetTextBlock()[0].d2dLayoutRect.left,
        GetTextBlock()[0].d2dLayoutRect.top,
        GetTextBlock()[0].d2dLayoutRect.right,
        GetTextBlock()[0].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[1].d2dLayoutRect.left,
        GetTextBlock()[1].d2dLayoutRect.top,
        GetTextBlock()[1].d2dLayoutRect.right,
        GetTextBlock()[1].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[2].d2dLayoutRect.left,
        GetTextBlock()[2].d2dLayoutRect.top,
        GetTextBlock()[2].d2dLayoutRect.right,
        GetTextBlock()[2].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[1].d2dLayoutRect.left,
        GetTextBlock()[1].d2dLayoutRect.top,
        GetTextBlock()[1].d2dLayoutRect.right,
        GetTextBlock()[1].d2dLayoutRect.bottom -
        (GetTextBlock()[1].d2dLayoutRect.top- GetTextBlock()[1].d2dLayoutRect.bottom)/4*3
        },
        {
        GetTextBlock()[2].d2dLayoutRect.left,
        GetTextBlock()[2].d2dLayoutRect.top,
        GetTextBlock()[2].d2dLayoutRect.right,
        GetTextBlock()[2].d2dLayoutRect.bottom -
        (GetTextBlock()[2].d2dLayoutRect.top - GetTextBlock()[2].d2dLayoutRect.bottom) / 4 * 3
        },
        {
        GetTextBlock()[3].d2dLayoutRect.left,
        GetTextBlock()[3].d2dLayoutRect.top,
        GetTextBlock()[3].d2dLayoutRect.right,
        GetTextBlock()[3].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[4].d2dLayoutRect.left,
        GetTextBlock()[4].d2dLayoutRect.top,
        GetTextBlock()[4].d2dLayoutRect.right,
        GetTextBlock()[4].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[5].d2dLayoutRect.left,
        GetTextBlock()[5].d2dLayoutRect.top,
        GetTextBlock()[5].d2dLayoutRect.right,
        GetTextBlock()[5].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[6].d2dLayoutRect.left,
        GetTextBlock()[6].d2dLayoutRect.top,
        GetTextBlock()[6].d2dLayoutRect.right,
        GetTextBlock()[6].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[7].d2dLayoutRect.left,
        GetTextBlock()[7].d2dLayoutRect.top,
        GetTextBlock()[7].d2dLayoutRect.right,
        GetTextBlock()[7].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[8].d2dLayoutRect.left,
        GetTextBlock()[8].d2dLayoutRect.top,
        GetTextBlock()[8].d2dLayoutRect.right,
        GetTextBlock()[8].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[9].d2dLayoutRect.left,
        GetTextBlock()[9].d2dLayoutRect.top,
        GetTextBlock()[9].d2dLayoutRect.right,
        GetTextBlock()[9].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[10].d2dLayoutRect.left,
        GetTextBlock()[10].d2dLayoutRect.top,
        GetTextBlock()[10].d2dLayoutRect.right,
        GetTextBlock()[10].d2dLayoutRect.bottom
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {
        GetTextBlock()[0].d2dLayoutRect.left,
        GetTextBlock()[0].d2dLayoutRect.top,
        GetTextBlock()[0].d2dLayoutRect.right,
        GetTextBlock()[0].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[1].d2dLayoutRect.left,
        GetTextBlock()[1].d2dLayoutRect.top,
        GetTextBlock()[1].d2dLayoutRect.right,
        GetTextBlock()[1].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[2].d2dLayoutRect.left,
        GetTextBlock()[2].d2dLayoutRect.top,
        GetTextBlock()[2].d2dLayoutRect.right,
        GetTextBlock()[2].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[1].d2dLayoutRect.left,
        GetTextBlock()[1].d2dLayoutRect.top,
        GetTextBlock()[1].d2dLayoutRect.right,
        GetTextBlock()[1].d2dLayoutRect.bottom -
        (GetTextBlock()[1].d2dLayoutRect.top - GetTextBlock()[1].d2dLayoutRect.bottom) / 4 * 3
        },
        {
        GetTextBlock()[2].d2dLayoutRect.left,
        GetTextBlock()[2].d2dLayoutRect.top,
        GetTextBlock()[2].d2dLayoutRect.right,
        GetTextBlock()[2].d2dLayoutRect.bottom -
        (GetTextBlock()[2].d2dLayoutRect.top - GetTextBlock()[2].d2dLayoutRect.bottom) / 4 * 3
        },
        {
        GetTextBlock()[3].d2dLayoutRect.left,
        GetTextBlock()[3].d2dLayoutRect.top,
        GetTextBlock()[3].d2dLayoutRect.right,
        GetTextBlock()[3].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[4].d2dLayoutRect.left,
        GetTextBlock()[4].d2dLayoutRect.top,
        GetTextBlock()[4].d2dLayoutRect.right,
        GetTextBlock()[4].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[5].d2dLayoutRect.left,
        GetTextBlock()[5].d2dLayoutRect.top,
        GetTextBlock()[5].d2dLayoutRect.right,
        GetTextBlock()[5].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[6].d2dLayoutRect.left,
        GetTextBlock()[6].d2dLayoutRect.top,
        GetTextBlock()[6].d2dLayoutRect.right,
        GetTextBlock()[6].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[7].d2dLayoutRect.left,
        GetTextBlock()[7].d2dLayoutRect.top,
        GetTextBlock()[7].d2dLayoutRect.right,
        GetTextBlock()[7].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[8].d2dLayoutRect.left,
        GetTextBlock()[8].d2dLayoutRect.top,
        GetTextBlock()[8].d2dLayoutRect.right,
        GetTextBlock()[8].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[9].d2dLayoutRect.left,
        GetTextBlock()[9].d2dLayoutRect.top,
        GetTextBlock()[9].d2dLayoutRect.right,
        GetTextBlock()[9].d2dLayoutRect.bottom
        },
        {
        GetTextBlock()[10].d2dLayoutRect.left,
        GetTextBlock()[10].d2dLayoutRect.top,
        GetTextBlock()[10].d2dLayoutRect.right,
        GetTextBlock()[10].d2dLayoutRect.bottom
        }
    };
    bool IsOutlined[13] = { true, true, true, true, true, true, true, true, true, true, true, true, true };
    UI::BeginDraw(nFrame); 
    UI::RectDraw(RectLTRB, FillLTRB, 0, IsOutlined);
    float aOpacities[2] = { 0.5f, 0.5f };
    UI::DrawBmp(RectLTRB, 0, 1, aOpacities);
    UI::TextDraw(nFrame, GetTextBlock());
    UI::EndDraw(nFrame);
}

void RoomUI::CreateFontFormat()
{
    std::vector<float> fFontSize;

    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);

    SetFontSize(fFontSize);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
     Fonts.push_back(L"Vladimir Script ∫∏≈Î");
     Fonts.push_back(L"πŸ≈¡√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");

     SetFonts(Fonts);

     std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
     TextAlignments.resize(GetTextCnt());
     TextAlignments[0] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[7] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[8] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[9] = DWRITE_TEXT_ALIGNMENT_CENTER;
     TextAlignments[10] = DWRITE_TEXT_ALIGNMENT_CENTER;
     SetTextAllignments(TextAlignments);


    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}

void RoomUI::SetTextRect()
{//Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 2 / 32,  GetFrameHeight() * 9 / 16,  GetFrameWidth() /4,  GetFrameHeight() *11/16 );
    GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 2 / 32, GetFrameHeight() * 23 / 32, GetFrameWidth() / 4, GetFrameHeight() * 27 / 32);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 26 / 32, GetFrameHeight() * 25 / 32, GetFrameWidth()*30/32, GetFrameHeight() * 29 / 32);
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 1 / 32, GetFrameHeight() * 1/ 32, GetFrameWidth()*7/32, GetFrameHeight()*8/32);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 8 / 32, GetFrameHeight() * 1 / 32, GetFrameWidth() * 14 / 32, GetFrameHeight() * 8 / 32);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 15 / 32, GetFrameHeight() * 1 / 32, GetFrameWidth() * 21 / 32, GetFrameHeight() * 8 / 32);
    GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 22 / 32, GetFrameHeight() * 1 / 32, GetFrameWidth() * 28 / 32, GetFrameHeight() * 8 / 32);
    GetTextBlock()[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 1 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 7 / 32, GetFrameHeight() * 16 / 32);
    GetTextBlock()[8].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 8 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 14 / 32, GetFrameHeight() * 16 / 32);
    GetTextBlock()[9].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 15 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 21 / 32, GetFrameHeight() * 16 / 32);
    GetTextBlock()[10].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 22 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 28 / 32, GetFrameHeight() * 16 / 32);
}

void RoomUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    //SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI:  StartBox, CarSelectBox[2], NicknameBox[8], MapSelectBox[2]
    std::vector<D2D1::ColorF> colorList;
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Red);
    colorList.push_back(D2D1::ColorF::Orange);
    colorList.push_back(D2D1::ColorF::Yellow);
    colorList.push_back(D2D1::ColorF::Green);
    colorList.push_back(D2D1::ColorF::Blue);
    colorList.push_back(D2D1::ColorF::Navy);
    colorList.push_back(D2D1::ColorF::Violet);
    colorList.push_back(D2D1::ColorF::Pink);
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Green, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f));


    //D2D1::ColorF colorList[24] = { D2D1::ColorF::Black, D2D1::ColorF::Black, D2D1::ColorF::Black, 
    //    D2D1::ColorF::Red, D2D1::ColorF::Orange, D2D1::ColorF::Yellow, D2D1::ColorF::Green, 
    //    D2D1::ColorF::Blue, D2D1::ColorF::Navy, D2D1::ColorF::Violet, D2D1::ColorF::Pink,//Text
    //D2D1::ColorF(D2D1::ColorF::Red, 0.5f), D2D1::ColorF(D2D1::ColorF::Blue, 0.5f), 
    //    D2D1::ColorF(D2D1::ColorF::Green, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
    //    D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
    //    D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
    //   D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
    //    D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
    //    D2D1::ColorF(D2D1::ColorF::Gray, 0.5f)/*UI*/
    //};
    SetColors(colorList);
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildSolidBrush(GetColors());

    SetTextRect(); 
    for (auto wc : std::wstring{ L"StartOrReady" })
        GetTextBlock()[0].strText.push_back(wc);
    for (auto wc : std::wstring{ L"CarSelect" })
        GetTextBlock()[1].strText.push_back(wc);
    for (auto wc : std::wstring{ L"MapSelect" })
        GetTextBlock()[2].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname1" })
        GetTextBlock()[3].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname2" })
        GetTextBlock()[4].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname3" })
        GetTextBlock()[5].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname4" })
        GetTextBlock()[6].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname5" })
        GetTextBlock()[7].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname6" })
        GetTextBlock()[8].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname7" })
        GetTextBlock()[9].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname8" })
        GetTextBlock()[10].strText.push_back(wc);
}

void RoomUI::Reset()
{
    UI::Reset();
    GetBitmapFileNames().clear();

}

void RoomUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    UI::Initialize(device, pd3dCommandQueue);
    //Reset();
    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
    BuildObjects(ppd3dRenderTargets, width, height);
}