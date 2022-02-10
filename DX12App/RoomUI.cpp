#include "stdafx.h"
#include "RoomUI.h"
RoomUI::RoomUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, pd3dDevice, pd3dCommandQueue), TextCnt(11), UICnt(13)
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI: NicknameBox[8], StartBox, CarSelectBox[2], MapSelectBox[2]
{

}
RoomUI::~RoomUI()
{

}

void RoomUI::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{

}

void RoomUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
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
    UI::BeginDraw(nFrame); 
    UI::RectDraw(RectLTRB, FillLTRB, TextCnt, 0, 0);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void RoomUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 15.0f;
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

    UI::CreateFontFormat(fFontSize, Fonts, TextCnt);
}

void RoomUI::SetTextRect()
{
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 3 / 8, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 2 / 8);
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 1 / 16, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 0.0f, mfWidth, mfHeight / 6);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 5 * (mfHeight / 6), mfWidth, mfHeight);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 8, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 7, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 3 / 8, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 2 / 8);
    mvTextBlocks[7].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 1 / 16, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2);
    mvTextBlocks[8].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 0.0f, mfWidth, mfHeight / 6);
    mvTextBlocks[9].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 5 * (mfHeight / 6), mfWidth, mfHeight);
    mvTextBlocks[10].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 8, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
}

void RoomUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[11] = { D2D1::ColorF::Black, D2D1::ColorF::Black, D2D1::ColorF::Black, D2D1::ColorF::Red, D2D1::ColorF::Orange, D2D1::ColorF::Yellow, D2D1::ColorF::Green, D2D1::ColorF::Blue, D2D1::ColorF::Navy, D2D1::ColorF::Violet, D2D1::ColorF::Pink };
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildSolidBrush(UICnt, TextCnt, colorList);

    SetTextRect();
}

void RoomUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void RoomUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}