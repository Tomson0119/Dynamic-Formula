#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue), TextCnt(13), UICnt(14)
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
}

LobbyUI::~LobbyUI()
{

}

void LobbyUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

}

bool LobbyUI::MouseCollisionCheck(float x, float y, const TextBlock& TB)
{
    if (x<TB.d2dLayoutRect.right && x>TB.d2dLayoutRect.left &&
        y<TB.d2dLayoutRect.bottom && y>TB.d2dLayoutRect.top)
        return true;
    return false;
}

void LobbyUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
    //mvd2dLinearGradientBrush.resize(TextCnt);
}

void LobbyUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    if (MouseCollisionCheck(dx, dy, mvTextBlocks[0]))
        mvColors[0].a = 0.1f;
    else
        mvColors[0].a = 0.9f;
    for (int i = 1; i < static_cast<int>(TextCnt); ++i)
    {// 12, 34, 56, 78, 910, 1112
        if (MouseCollisionCheck(dx, dy, mvTextBlocks[i]))
        {
            mvColors[(i+1)/2*2-1].a = 0.1f;
        }
        else mvColors[i].a = 0.9f;
    }
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);

}

char LobbyUI::OnProcessMouseDown(HWND hwnd, WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    for (int i = 1; i < static_cast<int>(TextCnt); ++i)
    {
        if (MouseCollisionCheck(dx, dy, mvTextBlocks[i]))
        {
           
            mvColors[(i+1)/2].a = 0.1f;
            if (buttonState)
                return i;
        }
        else mvColors[i].a = 0.9f;
    }
    return 66;
}

void LobbyUI::Update(float GTime)
{

}

void LobbyUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {
        mfWidth / 32 * 8,
        mfHeight / 36,
        mfWidth / 32 * 24,
        mfHeight / 36 * 29
        },
        {
            mfWidth / 32 * 9, 
            mfHeight / 18, 
            mfWidth / 32 * 15, 
            mfHeight / 18 * 2
        },
        {
        mfWidth / 32 * 9,
        mfHeight / 36 * 10, 
        mfWidth / 32 * 15, 
        mfHeight / 36 * 12
        },
        {
            mfWidth / 32 * 16, 
            mfHeight / 36 * 10, 
            mfWidth / 32 * 22, 
            mfHeight / 36 * 12
        },
        {
            mfWidth / 32 * 9, 
            mfHeight / 36 * 16,
            mfWidth / 32 * 15, 
            mfHeight / 36 * 18
        },
        {
        mfWidth / 32 * 16,
        mfHeight / 36 * 16,
        mfWidth / 32 * 22, 
        mfHeight / 36 * 18
        },
        {
        mfWidth / 32 * 9, 
        mfHeight / 36 * 22, 
        mfWidth / 32 * 15, 
        mfHeight / 36 * 24
        },
        {
        mfWidth / 32 * 16, 
        mfHeight / 36 * 22,
        mfWidth / 32 * 22, 
        mfHeight / 36 * 24
        },
        //---
        {
        mfWidth / 32 * 8,
        mfHeight / 36 * 9,
        mfWidth / 32 * 16,
        mfHeight / 36 * 15
        },
        {
            mfWidth / 32 * 17,
            mfHeight / 36 * 9,
            mfWidth / 32 * 25,
            mfHeight / 36 * 15
        },
        {
            mfWidth / 32 * 8,
            mfHeight / 36 * 15,
            mfWidth / 32 * 16,
            mfHeight / 36 * 21
        },
        {
        mfWidth / 32 * 17,
        mfHeight / 36 * 15,
        mfWidth / 32 * 25,
        mfHeight / 36 * 21
        },
        {
        mfWidth / 32 * 8,
        mfHeight / 36 * 21,
        mfWidth / 32 * 16,
        mfHeight / 36 * 27
        },
        {
        mfWidth / 32 * 17,
        mfHeight / 36 * 21,
        mfWidth / 32 * 25,
        mfHeight / 36 * 27
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {
        mfWidth / 32 * 8,
        mfHeight / 36,
        mfWidth / 32 * 24,
        mfHeight / 36 * 29
        },
        {
            mfWidth / 32 * 9,
            mfHeight / 18,
            mfWidth / 32 * 15,
            mfHeight / 18 * 2
        },
        {
        mfWidth / 32 * 9,
        mfHeight / 36 * 10,
        mfWidth / 32 * 15,
        mfHeight / 36 * 12
        },
        {
            mfWidth / 32 * 16,
            mfHeight / 36 * 10,
            mfWidth / 32 * 22,
            mfHeight / 36 * 12
        },
        {
            mfWidth / 32 * 9,
            mfHeight / 36 * 16,
            mfWidth / 32 * 15,
            mfHeight / 36 * 18
        },
        {
        mfWidth / 32 * 16,
        mfHeight / 36 * 16,
        mfWidth / 32 * 22,
        mfHeight / 36 * 18
        },
        {
        mfWidth / 32 * 9,
        mfHeight / 36 * 22,
        mfWidth / 32 * 15,
        mfHeight / 36 * 24
        },
        {
        mfWidth / 32 * 16,
        mfHeight / 36 * 22,
        mfWidth / 32 * 22,
        mfHeight / 36 * 24
        },
        {
        mfWidth / 32 * 8,
        mfHeight / 36 * 9,
        mfWidth / 32 * 16,
        mfHeight / 36 * 15
        },
        {
            mfWidth / 32 * 17,
            mfHeight / 36 * 9,
            mfWidth / 32 * 25,
            mfHeight / 36 * 15
        },
        {
            mfWidth / 32 * 8,
            mfHeight / 36 * 15,
            mfWidth / 32 * 16,
            mfHeight / 36 * 21
        },
        {
        mfWidth / 32 * 17,
        mfHeight / 36 * 15,
        mfWidth / 32 * 25,
        mfHeight / 36 * 21
        },
        {
        mfWidth / 32 * 8,
        mfHeight / 36 * 21,
        mfWidth / 32 * 16,
        mfHeight / 36 * 27
        },
        {
        mfWidth / 32 * 17,
        mfHeight / 36 * 21,
        mfWidth / 32 * 25,
        mfHeight / 36 * 27
        }
    };
    UI::BeginDraw(nFrame);
    UI::RoundedRectDraw(RectLTRB, FillLTRB, TextCnt + 1, 0, 0);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LobbyUI::CreateFontFormat()
{
    fFontSize = mfHeight / 25.0f;
    Fonts.push_back(L"Tahoma");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼"); 
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");
    Fonts.push_back(L"±¼¸²Ã¼");

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
    TextAlignments[11] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[12] = DWRITE_TEXT_ALIGNMENT_CENTER;

    UI::CreateFontFormat(fFontSize, Fonts, TextCnt, TextAlignments);
}

void LobbyUI::SetTextRect()
{//MakeRoom, Room1, Room2, Room3, Room4, Room5, Room6
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 9, mfHeight / 18, mfWidth / 32 * 15, mfHeight / 18 * 2);

    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 9, mfHeight / 36 * 10, mfWidth / 32 * 15, mfHeight / 36 * 12);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 13, mfHeight / 36 * 13, mfWidth / 32 * 15, mfHeight / 36 * 14);

    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 16, mfHeight / 36 * 10, mfWidth / 32 * 22, mfHeight / 36 * 12);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 20, mfHeight / 36 * 13, mfWidth / 32 * 22, mfHeight / 36 * 14);

    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 9, mfHeight / 36 * 16, mfWidth / 32 * 15, mfHeight / 36 * 18);
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 13, mfHeight / 36 * 19, mfWidth / 32 * 15, mfHeight / 36 * 20);

    mvTextBlocks[7].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 16, mfHeight / 36 * 16, mfWidth / 32 * 22, mfHeight / 36 * 18);
    mvTextBlocks[8].d2dLayoutRect = D2D1::RectF(mfWidth / 32 *20, mfHeight / 36 * 19, mfWidth / 32 * 22, mfHeight / 36 * 20);

    mvTextBlocks[9].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 9, mfHeight / 36 * 22, mfWidth / 32 * 15, mfHeight / 36 * 24);
    mvTextBlocks[10].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 13, mfHeight / 36 * 25, mfWidth / 32 * 15, mfHeight / 36 * 26);

    mvTextBlocks[11].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 16, mfHeight / 36 * 22, mfWidth / 32 * 22, mfHeight / 36 * 24);
    mvTextBlocks[12].d2dLayoutRect = D2D1::RectF(mfWidth / 32 * 20, mfHeight / 36 * 25, mfWidth / 32 * 22, mfHeight / 36 * 26);

}

void LobbyUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[27] = { D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) };
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    for (auto color : colorList)
        mvColors.push_back(color);
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);

    SetTextRect();
    for (auto wc : std::wstring{ L"MakeRoom" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room No.1" })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::wstring{ L"1" })
        mvTextBlocks[2].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room Title 2" })
        mvTextBlocks[3].strText.push_back(wc);
    for (auto wc : std::wstring{ L"2" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::wstring{ L"0/03" })
        mvTextBlocks[5].strText.push_back(wc);
    for (auto wc : std::wstring{ L"3" })
        mvTextBlocks[6].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room No. 4" })
        mvTextBlocks[7].strText.push_back(wc);
    for (auto wc : std::wstring{ L"4" })
        mvTextBlocks[8].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room Title 5" })
        mvTextBlocks[9].strText.push_back(wc);
    for (auto wc : std::wstring{ L"5" })
        mvTextBlocks[10].strText.push_back(wc);
    for (auto wc : std::wstring{ L"0 / 0 6" })
        mvTextBlocks[11].strText.push_back(wc);
    for (auto wc : std::wstring{ L"6" })
        mvTextBlocks[12].strText.push_back(wc);
}

void LobbyUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void LobbyUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    UI::Initialize(device, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}