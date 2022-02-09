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
    mvTextBlocks.resize(TextCnt);
    //mvd2dLinearGradientBrush.resize(TextCnt);
}

void InGameUI::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{

}
void InGameUI::StartPrint(const std::wstring& strUIText)
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
    XMFLOAT4 RectLTRB[] =
    {
        {
            mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f),
        mfWidth * (3.0f / 16.0f) + (mfWidth * (1.0f / 2.0f) - mfWidth * (3.0f / 16.0f)),
        mfHeight * (8.0f / 9.0f)
        }, //DraftGage
        {
            mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f),
        mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }, //Item1 UI
        {
            mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f),
        mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }//Item2 UI
    };
    XMFLOAT4 FillLTRB[] = 
    { 
        {
            mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (3.0f / 16.0f) + (mfWidth * (1.0f / 2.0f) - mfWidth * (3.0f / 16.0f)) * fDraftGage, 
        mfHeight * (8.0f / 9.0f)
        }, //DraftGage
        {
            mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }, //Item1 UI
        {
            mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }//Item2 UI
    };
    UI::Draw(nFrame, TextCnt, 1, mvTextBlocks, RectLTRB, FillLTRB);
}

void InGameUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 15.0f;
    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
    Fonts.push_back(L"Vladimir Script ∫∏≈Î");
    Fonts.push_back(L"πŸ≈¡√º");
    Fonts.push_back(L"±º∏≤√º");
    Fonts.push_back(L"±º∏≤√º");

    UI::CreateFontFormat(fFontSize, Fonts);         
}

void InGameUI::SetTextRect()
{
     mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(0.0f, 23.0f + mfHeight / 6, mfWidth / 6, 23.0f + (mfHeight / 6));
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(0.0f, 23.0f, mfWidth / 6, mfHeight / 6);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 0.0f, mfWidth, mfHeight / 6);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 5 * (mfHeight / 6), mfWidth, mfHeight);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 8, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
}

void InGameUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[8] = { D2D1::ColorF::Black, (0xE12C38, 1.0f), (0xE12C38, 1.0f), D2D1::ColorF::Black, D2D1::ColorF::OrangeRed, D2D1::ColorF::Yellow, D2D1::ColorF::Red, D2D1::ColorF::Aqua };
    D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildBrush(UICnt, TextCnt, colorList, 4, gradientColors);
    
    SetTextRect();
}

void InGameUI::Flush()
{
    
}

void InGameUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void InGameUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    Initialize(pd3dDevice, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}