
#include "stdafx.h"
#include "LoginUI.h"
LoginUI::LoginUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, pd3dDevice, pd3dCommandQueue) , TextCnt(6), UICnt(1)
    //Text: GameName, Login, ID, IDText, Password, PassWordText
    //UI: LoginBox
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
}

LoginUI::~LoginUI()
{

}

void LoginUI::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{

}
void LoginUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
    //mvd2dLinearGradientBrush.resize(TextCnt);
}
void LoginUI::Update(float GTime)
{
    for (int i = 0; i < 6; ++i)
        mvTextBlocks[i].strText.clear();
    for (auto wc : std::wstring{ L"Dynamic Fomula" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Login" })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::wstring{ L"ID" })
        mvTextBlocks[2].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Password" })
        mvTextBlocks[3].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Password" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::wstring{ L"Password" })
        mvTextBlocks[5].strText.push_back(wc);
}

void LoginUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {
        mvTextBlocks[0].d2dLayoutRect.left,
        mvTextBlocks[0].d2dLayoutRect.top,
        mvTextBlocks[0].d2dLayoutRect.right,
        mvTextBlocks[0].d2dLayoutRect.bottom
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {
        mvTextBlocks[0].d2dLayoutRect.left,
        mvTextBlocks[0].d2dLayoutRect.top,
        mvTextBlocks[0].d2dLayoutRect.right,
        mvTextBlocks[0].d2dLayoutRect.bottom
        }
    };
    UI::BeginDraw(nFrame);
    UI::RectDraw(RectLTRB, FillLTRB, TextCnt, 0, 0);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LoginUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 15.0f;
    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
    Fonts.push_back(L"Vladimir Script ∫∏≈Î");
    Fonts.push_back(L"πŸ≈¡√º");
    Fonts.push_back(L"±º∏≤√º");
    Fonts.push_back(L"±º∏≤√º");
    Fonts.push_back(L"±º∏≤√º");


    UI::CreateFontFormat(fFontSize, Fonts, TextCnt);
}

void LoginUI::SetTextRect()
{
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth/2 - mfWidth*3/32, mfHeight/2 - mfHeight*3/8, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 2/ 8);
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth / 2 - mfWidth * 3 / 32, mfHeight / 2 - mfHeight * 1 / 16, mfWidth / 2 + mfWidth * 3 / 32, mfHeight / 2);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 0.0f, mfWidth, mfHeight / 6);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 5 * (mfHeight / 6), mfWidth, mfHeight);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 8, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 7, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
}

void LoginUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[7] = { (D2D1::ColorF::Black, 1.0f), (D2D1::ColorF::Black, 1.0f), (D2D1::ColorF::Black, 1.0f), (D2D1::ColorF::Black, 1.0f), (D2D1::ColorF::Black, 1.0f), D2D1::ColorF(0xFF0000, 0.1f), D2D1::ColorF(D2D1::ColorF::Red, 0.1f) };
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildSolidBrush(UICnt, TextCnt, colorList);

    SetTextRect();
}

void LoginUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void LoginUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}