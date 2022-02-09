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

}

void RoomUI::Draw(UINT nFrame)
{
    //UI::Draw(nFrame, TextCnt, 1, mvTextBlocks, RectLTRB, FillLTRB);
}

void RoomUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 15.0f;
    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
    /* Fonts.push_back(L"Vladimir Script ∫∏≈Î");
     Fonts.push_back(L"πŸ≈¡√º");
     Fonts.push_back(L"±º∏≤√º");
     Fonts.push_back(L"±º∏≤√º");*/

    UI::CreateFontFormat(fFontSize, Fonts, TextCnt);
}

void RoomUI::SetTextRect()
{

}

void RoomUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
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