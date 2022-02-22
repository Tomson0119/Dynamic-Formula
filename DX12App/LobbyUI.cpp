#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue), TextCnt(7), UICnt()
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

void LobbyUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
    //mvd2dLinearGradientBrush.resize(TextCnt);
}

void LobbyUI::Update(float GTime)
{

}

void LobbyUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {
        mvTextBlocks[2].d2dLayoutRect.left,
        mvTextBlocks[0].d2dLayoutRect.top,
        mvTextBlocks[5].d2dLayoutRect.right,
        mvTextBlocks[5].d2dLayoutRect.bottom
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {
        mvTextBlocks[2].d2dLayoutRect.left,
        mvTextBlocks[0].d2dLayoutRect.top,
        mvTextBlocks[5].d2dLayoutRect.right,
        mvTextBlocks[5].d2dLayoutRect.bottom
        }
    };
    UI::BeginDraw(nFrame);
    UI::RectDraw(RectLTRB, FillLTRB, TextCnt + 1, 0, 0);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LobbyUI::CreateFontFormat()
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



    UI::CreateFontFormat(fFontSize, Fonts, TextCnt, DWRITE_TEXT_ALIGNMENT_LEADING);
}

void LobbyUI::SetTextRect()
{//MakeRoom, Room1, Room2, Room3, Room4, Room5, Room6
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 2 / 32, mfHeight / 9, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 9 * 2);
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 2 / 32, mfHeight / 2 - mfHeight * 1 / 16, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(mfWidth / 2 + mfWidth * 6 / 32, 4 * (mfHeight / 6), mfWidth / 2 + mfWidth * 9 / 32, 9 * (mfHeight / 12));
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 2 / 32, 4 * (mfHeight / 6), mfWidth / 2 + mfWidth * 3 / 32, 9 * (mfHeight / 12));
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth / 2 + mfWidth * 6 / 32, mfWidth / 2 + mfWidth * 9 / 32, 11 * (mfHeight / 12));
    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 2 / 32, 5 * (mfHeight / 6), mfWidth / 2 + mfWidth * 3 / 32, 11 * (mfHeight / 12));
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(mfWidth / 2 + mfWidth * 6 / 32, 5 * (mfHeight / 6), mfWidth / 2 + mfWidth * 9 / 32, 11 * (mfHeight / 12));
}
void LobbyUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[8] = { D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Blue, 0.1f), D2D1::ColorF(D2D1::ColorF::Blue, 0.1f) };
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildSolidBrush(UICnt + 1, TextCnt, colorList);

    SetTextRect();
    for (auto wc : std::wstring{ L"MakeRoom" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room No." })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room Title" })
        mvTextBlocks[2].strText.push_back(wc);
    for (auto wc : std::wstring{ L"0 / 0" })
        mvTextBlocks[3].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room No." })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Room Title" })
        mvTextBlocks[5].strText.push_back(wc);
    for (auto wc : std::wstring{ L"0 / 0" })
        mvTextBlocks[6].strText.push_back(wc);
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