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
    SetVectorSize(nFrame, GetTextCnt());
    Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < mvBitmapFileNames.size(); ++i)
        LoadBitmapResourceFromFile(mvBitmapFileNames[i], i);
}

RoomUI::~RoomUI()
{

}

void RoomUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

}

void RoomUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
    SetBitmapsSize(2);


    mvBitmapFileNames.push_back(L"Resources\\SampleImg.jpg");
    mvBitmapFileNames.push_back(L"Resources\\SampleImg.jpg");

    //mvd2dLinearGradientBrush.resize(TextCnt);
}

void RoomUI::Update(float GTime)
{
    for (int i = 0; i < 11; ++i)
        mvTextBlocks[i].strText.clear();
    for (auto wc : std::wstring{ L"StartOrReady" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::wstring{ L"CarSelect" })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::wstring{ L"MapSelect" })
        mvTextBlocks[2].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname1" })
        mvTextBlocks[3].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname2" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname3" })
        mvTextBlocks[5].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname4" })
        mvTextBlocks[6].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname5" })
        mvTextBlocks[7].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname6" })
        mvTextBlocks[8].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname7" })
        mvTextBlocks[9].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname8" })
        mvTextBlocks[10].strText.push_back(wc);
    
}

void RoomUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {
        mvTextBlocks[0].d2dLayoutRect.left,
        mvTextBlocks[0].d2dLayoutRect.top,
        mvTextBlocks[0].d2dLayoutRect.right,
        mvTextBlocks[0].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[1].d2dLayoutRect.left,
        mvTextBlocks[1].d2dLayoutRect.top,
        mvTextBlocks[1].d2dLayoutRect.right,
        mvTextBlocks[1].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[2].d2dLayoutRect.left,
        mvTextBlocks[2].d2dLayoutRect.top,
        mvTextBlocks[2].d2dLayoutRect.right,
        mvTextBlocks[2].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[1].d2dLayoutRect.left,
        mvTextBlocks[1].d2dLayoutRect.top,
        mvTextBlocks[1].d2dLayoutRect.right,
        mvTextBlocks[1].d2dLayoutRect.bottom - 
        (mvTextBlocks[1].d2dLayoutRect.top- mvTextBlocks[1].d2dLayoutRect.bottom)/4*3
        },
        {
        mvTextBlocks[2].d2dLayoutRect.left,
        mvTextBlocks[2].d2dLayoutRect.top,
        mvTextBlocks[2].d2dLayoutRect.right,
        mvTextBlocks[2].d2dLayoutRect.bottom -
        (mvTextBlocks[2].d2dLayoutRect.top - mvTextBlocks[2].d2dLayoutRect.bottom) / 4 * 3
        },
        {
        mvTextBlocks[3].d2dLayoutRect.left,
        mvTextBlocks[3].d2dLayoutRect.top,
        mvTextBlocks[3].d2dLayoutRect.right,
        mvTextBlocks[3].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[4].d2dLayoutRect.left,
        mvTextBlocks[4].d2dLayoutRect.top,
        mvTextBlocks[4].d2dLayoutRect.right,
        mvTextBlocks[4].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[5].d2dLayoutRect.left,
        mvTextBlocks[5].d2dLayoutRect.top,
        mvTextBlocks[5].d2dLayoutRect.right,
        mvTextBlocks[5].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[6].d2dLayoutRect.left,
        mvTextBlocks[6].d2dLayoutRect.top,
        mvTextBlocks[6].d2dLayoutRect.right,
        mvTextBlocks[6].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[7].d2dLayoutRect.left,
        mvTextBlocks[7].d2dLayoutRect.top,
        mvTextBlocks[7].d2dLayoutRect.right,
        mvTextBlocks[7].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[8].d2dLayoutRect.left,
        mvTextBlocks[8].d2dLayoutRect.top,
        mvTextBlocks[8].d2dLayoutRect.right,
        mvTextBlocks[8].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[9].d2dLayoutRect.left,
        mvTextBlocks[9].d2dLayoutRect.top,
        mvTextBlocks[9].d2dLayoutRect.right,
        mvTextBlocks[9].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[10].d2dLayoutRect.left,
        mvTextBlocks[10].d2dLayoutRect.top,
        mvTextBlocks[10].d2dLayoutRect.right,
        mvTextBlocks[10].d2dLayoutRect.bottom
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {
        mvTextBlocks[0].d2dLayoutRect.left,
        mvTextBlocks[0].d2dLayoutRect.top,
        mvTextBlocks[0].d2dLayoutRect.right,
        mvTextBlocks[0].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[1].d2dLayoutRect.left,
        mvTextBlocks[1].d2dLayoutRect.top,
        mvTextBlocks[1].d2dLayoutRect.right,
        mvTextBlocks[1].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[2].d2dLayoutRect.left,
        mvTextBlocks[2].d2dLayoutRect.top,
        mvTextBlocks[2].d2dLayoutRect.right,
        mvTextBlocks[2].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[1].d2dLayoutRect.left,
        mvTextBlocks[1].d2dLayoutRect.top,
        mvTextBlocks[1].d2dLayoutRect.right,
        mvTextBlocks[1].d2dLayoutRect.bottom -
        (mvTextBlocks[1].d2dLayoutRect.top - mvTextBlocks[1].d2dLayoutRect.bottom) / 4 * 3
        },
        {
        mvTextBlocks[2].d2dLayoutRect.left,
        mvTextBlocks[2].d2dLayoutRect.top,
        mvTextBlocks[2].d2dLayoutRect.right,
        mvTextBlocks[2].d2dLayoutRect.bottom -
        (mvTextBlocks[2].d2dLayoutRect.top - mvTextBlocks[2].d2dLayoutRect.bottom) / 4 * 3
        },
        {
        mvTextBlocks[3].d2dLayoutRect.left,
        mvTextBlocks[3].d2dLayoutRect.top,
        mvTextBlocks[3].d2dLayoutRect.right,
        mvTextBlocks[3].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[4].d2dLayoutRect.left,
        mvTextBlocks[4].d2dLayoutRect.top,
        mvTextBlocks[4].d2dLayoutRect.right,
        mvTextBlocks[4].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[5].d2dLayoutRect.left,
        mvTextBlocks[5].d2dLayoutRect.top,
        mvTextBlocks[5].d2dLayoutRect.right,
        mvTextBlocks[5].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[6].d2dLayoutRect.left,
        mvTextBlocks[6].d2dLayoutRect.top,
        mvTextBlocks[6].d2dLayoutRect.right,
        mvTextBlocks[6].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[7].d2dLayoutRect.left,
        mvTextBlocks[7].d2dLayoutRect.top,
        mvTextBlocks[7].d2dLayoutRect.right,
        mvTextBlocks[7].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[8].d2dLayoutRect.left,
        mvTextBlocks[8].d2dLayoutRect.top,
        mvTextBlocks[8].d2dLayoutRect.right,
        mvTextBlocks[8].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[9].d2dLayoutRect.left,
        mvTextBlocks[9].d2dLayoutRect.top,
        mvTextBlocks[9].d2dLayoutRect.right,
        mvTextBlocks[9].d2dLayoutRect.bottom
        },
        {
        mvTextBlocks[10].d2dLayoutRect.left,
        mvTextBlocks[10].d2dLayoutRect.top,
        mvTextBlocks[10].d2dLayoutRect.right,
        mvTextBlocks[10].d2dLayoutRect.bottom
        }
    };
    bool IsOutlined[13] = { true, true, true, true, true, true, true, true, true, true, true, true, true };
    UI::BeginDraw(nFrame); 
    UI::RectDraw(RectLTRB, FillLTRB, GetTextCnt() + 1, 0, 0, IsOutlined);
    float aOpacities[2] = { 0.5f, 0.5f };
    UI::DrawBmp(RectLTRB, 0, 1, aOpacities);
    UI::TextDraw(nFrame, GetTextCnt(), mvTextBlocks);
    UI::EndDraw(nFrame);
}

void RoomUI::CreateFontFormat()
{
    float fFontSize = GetFrameHeight() / 25.0f;
    //vfFontSize.resize(TextCnt);

    /*vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);
    vfFontSize.push_back(GetFrameHeight() / 15.0f);*/

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

     DWRITE_TEXT_ALIGNMENT TextAlignments[11];
     //TextAlignments.resize(TextCnt);
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
     


    UI::CreateFontFormat(fFontSize, Fonts, GetTextCnt(), TextAlignments);
}

void RoomUI::SetTextRect()
{//Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 2 / 32,  GetFrameHeight() * 9 / 16,  GetFrameWidth() /4,  GetFrameHeight() *11/16 );
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 2 / 32, GetFrameHeight() * 23 / 32, GetFrameWidth() / 4, GetFrameHeight() * 27 / 32);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 26 / 32, GetFrameHeight() * 25 / 32, GetFrameWidth()*30/32, GetFrameHeight() * 29 / 32);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 1 / 32, GetFrameHeight() * 1/ 32, GetFrameWidth()*7/32, GetFrameHeight()*8/32);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 8 / 32, GetFrameHeight() * 1 / 32, GetFrameWidth() * 14 / 32, GetFrameHeight() * 8 / 32);
    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 15 / 32, GetFrameHeight() * 1 / 32, GetFrameWidth() * 21 / 32, GetFrameHeight() * 8 / 32);
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 22 / 32, GetFrameHeight() * 1 / 32, GetFrameWidth() * 28 / 32, GetFrameHeight() * 8 / 32);
    mvTextBlocks[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 1 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 7 / 32, GetFrameHeight() * 16 / 32);
    mvTextBlocks[8].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 8 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 14 / 32, GetFrameHeight() * 16 / 32);
    mvTextBlocks[9].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 15 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 21 / 32, GetFrameHeight() * 16 / 32);
    mvTextBlocks[10].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 22 / 32, GetFrameHeight() * 9 / 32, GetFrameWidth() * 28 / 32, GetFrameHeight() * 16 / 32);
}

void RoomUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    //SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI:  StartBox, CarSelectBox[2], NicknameBox[8], MapSelectBox[2]
    D2D1::ColorF colorList[24] = { D2D1::ColorF::Black, D2D1::ColorF::Black, D2D1::ColorF::Black, 
        D2D1::ColorF::Red, D2D1::ColorF::Orange, D2D1::ColorF::Yellow, D2D1::ColorF::Green, 
        D2D1::ColorF::Blue, D2D1::ColorF::Navy, D2D1::ColorF::Violet, D2D1::ColorF::Pink,//Text
    D2D1::ColorF(D2D1::ColorF::Red, 0.5f), D2D1::ColorF(D2D1::ColorF::Blue, 0.5f), 
        D2D1::ColorF(D2D1::ColorF::Green, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
        D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
        D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
       D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
        D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), D2D1::ColorF(D2D1::ColorF::Gray, 0.5f),
        D2D1::ColorF(D2D1::ColorF::Gray, 0.5f)/*UI*/
    };
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildSolidBrush(GetRoundRectCnt(), GetTextCnt() + 1, colorList);

    SetTextRect(); 
    for (auto wc : std::wstring{ L"StartOrReady" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::wstring{ L"CarSelect" })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::wstring{ L"MapSelect" })
        mvTextBlocks[2].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname1" })
        mvTextBlocks[3].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname2" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname3" })
        mvTextBlocks[5].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname4" })
        mvTextBlocks[6].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname5" })
        mvTextBlocks[7].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname6" })
        mvTextBlocks[8].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname7" })
        mvTextBlocks[9].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Nickname8" })
        mvTextBlocks[10].strText.push_back(wc);
}

void RoomUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
    mvBitmapFileNames.clear();

}

void RoomUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, GetTextCnt());
    UI::Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < mvBitmapFileNames.size(); ++i)
        LoadBitmapResourceFromFile(mvBitmapFileNames[i], i);
    PreDraw(ppd3dRenderTargets, width, height);
}