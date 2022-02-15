#include "stdafx.h"
#include "RoomUI.h"
RoomUI::RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue), TextCnt(11), UICnt(13)
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI: NicknameBox[8], StartBox, CarSelectBox[2], MapSelectBox[2]
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
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
    UI::BeginDraw(nFrame); 
    UI::RectDraw(RectLTRB, FillLTRB, TextCnt+1, 0, 0);
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
{//Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth * 2 / 32,  mfHeight * 9 / 16,  mfWidth /4,  mfHeight *11/16 );
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth * 2 / 32, mfHeight * 23 / 32, mfWidth / 4, mfHeight * 27 / 32);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(mfWidth * 26 / 32, mfHeight * 25 / 32, mfWidth*30/32, mfHeight * 29 / 32);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 32, mfHeight * 1/ 32, mfWidth*7/32, mfHeight*8/32);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 8 / 32, mfHeight * 1 / 32, mfWidth * 14 / 32, mfHeight * 8 / 32);
    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth * 15 / 32, mfHeight * 1 / 32, mfWidth * 21 / 32, mfHeight * 8 / 32);
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(mfWidth * 22 / 32, mfHeight * 1 / 32, mfWidth * 28 / 32, mfHeight * 8 / 32);
    mvTextBlocks[7].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 32, mfHeight * 9 / 32, mfWidth * 7 / 32, mfHeight * 16 / 32);
    mvTextBlocks[8].d2dLayoutRect = D2D1::RectF(mfWidth * 8 / 32, mfHeight * 9 / 32, mfWidth * 14 / 32, mfHeight * 16 / 32);
    mvTextBlocks[9].d2dLayoutRect = D2D1::RectF(mfWidth * 15 / 32, mfHeight * 9 / 32, mfWidth * 21 / 32, mfHeight * 16 / 32);
    mvTextBlocks[10].d2dLayoutRect = D2D1::RectF(mfWidth * 22 / 32, mfHeight * 9 / 32, mfWidth * 28 / 32, mfHeight * 16 / 32);
}

void RoomUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
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
    UI::BuildSolidBrush(UICnt, TextCnt+1, colorList);

    SetTextRect();
}

void RoomUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void RoomUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}