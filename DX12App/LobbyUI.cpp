#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue), TextCnt(13), UICnt(18)
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
    for (int i = 0;i<mvBitmapFileNames.size();++i)
        LoadBitmapResourceFromFile(mvBitmapFileNames[i], i);
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
    SetBitmapsSize(3);

    mvBitmapFileNames.push_back(L"Resources\\SampleImg.jpg");
    mvBitmapFileNames.push_back(L"Resources\\YellowBackGroundFlag.jpeg");
    mvBitmapFileNames.push_back(L"Resources\\LeftRightArrow.jpeg");

    //mvd2dLinearGradientBrush.resize(TextCnt);
    SetGradientCnt(12);
}

int LobbyUI::OnProcessMouseClick(WPARAM buttonState, int x, int y)
{
    return 0;
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
            mvColors[(((i+1)/2)*2)-1].a = 0.1f;
        }
        else mvColors[i].a = 0.9f;
    }
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);

}

void LobbyUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    for (int i = 1; i < static_cast<int>(TextCnt); ++i)
    {
        if (MouseCollisionCheck(dx, dy, mvTextBlocks[i]))
        {
           
            mvColors[(i+1)/2].a = 0.1f;
           /* if (buttonState)
                return i;*/
        }
        else mvColors[i].a = 0.9f;
    }
    //return 66;
}

void LobbyUI::Update(float GTime)
{

}

void LobbyUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {//BigBackGroundBox
            mfWidth * 0.04f,
            mfHeight * 0.04f,
            mfWidth * 0.96f,
            mfHeight * 0.96f
        },
        {//SmallBackGroundBox
            mfWidth * 0.24f,
            mfHeight * 0.34f,
            mfWidth * 0.90f,
            mfHeight * 0.86f
        },
        {
        mfWidth * 0.25f,
        mfHeight * 0.11f,
        mfWidth * 0.75f,
        mfHeight * 0.77f
        },
        {
            mfWidth * 0.27f, 
            mfHeight * 0.055f, 
            mfWidth * 0.48f, 
            mfHeight * 0.11f
        },
        {
        mfWidth * 0.27f,
        mfHeight * 0.275f, 
        mfWidth * 0.48f, 
        mfHeight * 0.44f
        },
        {
            mfWidth * 0.5f, 
            mfHeight * 0.275f, 
            mfWidth * 0.61f, 
            mfHeight * 0.33f
        },
        {
            mfWidth * 0.275f, 
            mfHeight * 0.44f,
            mfWidth * 0.43f, 
            mfHeight * 0.5f
        },
        {
        mfWidth * 0.5f,
        mfHeight * 0.44f,
        mfWidth * 0.61f, 
        mfHeight * 0.5f
        },
        {
        mfWidth * 0.275f, 
        mfHeight * 0.61f, 
        mfWidth * 0.49f, 
        mfHeight * 0.66f
        },
        {
        mfWidth * 0.5f, 
        mfHeight * 0.61f,
        mfWidth * 0.61f, 
        mfHeight * 0.66f
        },
        //---
        {
        mfWidth * 0.22f,
        mfHeight * 0.23f,
        mfWidth * 0.5f,
        mfHeight * 0.42f
        },
        {
            mfWidth * 0.52f,
            mfHeight * 0.24f,
            mfWidth * 0.68f,
            mfHeight * 0.42f
        },
        {
            mfWidth * 0.25f,
            mfHeight * 0.42f,
            mfWidth * 0.5f,
            mfHeight * 0.57f
        },
        {
        mfWidth * 0.57f,
        mfHeight * 0.42f,
        mfWidth *0.78125f,
        mfHeight * 0.57f
        },
        {
        mfWidth * 0.25f,
        mfHeight * 0.57f,
        mfWidth * 0.5f,
        mfHeight *0.75f
        },
        {
        mfWidth * 0.52f,
        mfHeight * 0.57f,
        mfWidth * 0.77f,
        mfHeight * 0.75f
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {//BigBackGroundBox
            mfWidth * 0.04f,
            mfHeight * 0.02f,
            mfWidth * 0.96f,
            mfHeight * 0.96f
        },
        {//SmallBackGroundBox
            mfWidth * 0.18f,
            mfHeight * 0.04f,
            mfWidth * 0.86f,
            mfHeight * 0.86f
        },
        {//SmallWhiteBackGroundBox
        mfWidth * 0.20f,
        mfHeight * 0.18f,
        mfWidth * 0.80f,
        mfHeight * 0.84f
        },
        {//MakeRoomBox
            mfWidth * 0.27f,
            mfHeight * 0.055f,
            mfWidth * 0.48f,
            mfHeight * 0.11f
        },
        {//RoomTitle1
        mfWidth * 0.27f,
        mfHeight * 0.26f,
        mfWidth * 0.48f,
        mfHeight * 0.33f
        },
        {//RoomTitle2
            mfWidth * 0.54f,
            mfHeight * 0.26f,
            mfWidth * 0.75f,
            mfHeight * 0.33f
        },
        {//RoomTitle3
            mfWidth * 0.27f,
            mfHeight * 0.44f,
            mfWidth * 0.48f,
            mfHeight * 0.5f
        },
        {//RoomTitle4
        mfWidth * 0.54f,
        mfHeight * 0.44f,
        mfWidth * 0.75f,
        mfHeight *0.5f
        },
        {//RoomTitle5
        mfWidth * 0.27f,
        mfHeight * 0.605f,
        mfWidth * 0.48f,
        mfHeight * 0.66f
        },
        {//RoomTitle6
        mfWidth * 0.54f,
        mfHeight * 0.605f,
        mfWidth * 0.75f,
        mfHeight * 0.66f
        },
        {//RoomBox1
        mfWidth * 0.25f,
        mfHeight * 0.25f,
        mfWidth * 0.5f,
        mfHeight * 0.42f
        },
        {//RoomBox2
            mfWidth * 0.52f,
            mfHeight * 0.25f,
            mfWidth * 0.77f,
            mfHeight * 0.42f
        },
        {//RoomBox3
            mfWidth * 0.25f,
            mfHeight * 0.42f,
            mfWidth * 0.5f,
            mfHeight * 0.57f
        },
        {//RoomBox4
        mfWidth * 0.52f,
        mfHeight * 0.42f,
        mfWidth * 0.77f,
        mfHeight * 0.57f
        },
        {//RoomBox5
        mfWidth * 0.25f,
        mfHeight * 0.57f,
        mfWidth * 0.5f,
        mfHeight * 0.75f
        },
        {//RoomBox6
        mfWidth * 0.52f,
        mfHeight * 0.57f,
        mfWidth * 0.77f,
        mfHeight * 0.75f
        },
        {//LeftArrowBox
        mfWidth * 0.044f,
        mfHeight * 0.076f,
        mfWidth * 0.05f,
        mfHeight * 0.082f
        },
        {//RightArrowBox
        mfWidth * 0.052f,
        mfHeight * 0.076f,
        mfWidth * 0.058f,
        mfHeight * 0.082f
        }
    };
    XMFLOAT4 LTRB[] =
    {
        {
            390.0f,
            290.0f,
            410.0f,
            310.0f
        },
        {
            0.0f,
            0.0f,
            mfWidth,
            mfHeight
        },
        {
             mfWidth * 0.44f,
           mfHeight * 0.76f,
            mfWidth * 0.58f,
            mfHeight * 0.82f
        }
    };
    bool IsOutlined[18] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false };
    float aOpacities[3] = { 0.5f, 1.0f, 0.5f };
    UI::BeginDraw(nFrame);
    UI::DrawBmp(LTRB, 0, 2, aOpacities);
    UI::RoundedRectDraw(RectLTRB, FillLTRB, TextCnt + 1, 0, 0, IsOutlined);
    UI::DrawBmp(LTRB, 2, 1, aOpacities);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LobbyUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 25.0f;;
   // vfFontSize.resize(TextCnt);

    /*vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);
    vfFontSize.push_back(mfHeight / 15.0f);*/

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
    
    D2D1::ColorF colorList[31] = { D2D1::ColorF(D2D1::ColorF::DarkGray , 1.0f), D2D1::ColorF(D2D1::ColorF::DarkGray , 1.0f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), 
        D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), 
        D2D1::ColorF(D2D1::ColorF::Gray, 0.98f), D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f),D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f),
        D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::LightGray, 0.3f) , D2D1::ColorF(D2D1::ColorF::LightGray, 0.3f)};
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    for (auto color : colorList)
        mvColors.push_back(color);
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);

    SetTextRect();
    for (auto wc : std::string{ "MakeRoom" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::string{ "Room No.1" })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::string{ "1" })
        mvTextBlocks[2].strText.push_back(wc);
    for (auto wc : std::string{ "Room Title 2" })
        mvTextBlocks[3].strText.push_back(wc);
    for (auto wc : std::string{ "2" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::string{ "0/03" })
        mvTextBlocks[5].strText.push_back(wc);
    for (auto wc : std::string{ "3" })
        mvTextBlocks[6].strText.push_back(wc);
    for (auto wc : std::string{ "Room No. 4" })
        mvTextBlocks[7].strText.push_back(wc);
    for (auto wc : std::string{"4" })
        mvTextBlocks[8].strText.push_back(wc);
    for (auto wc : std::string{ "Room Title 5" })
        mvTextBlocks[9].strText.push_back(wc);
    for (auto wc : std::string{ "5" })
        mvTextBlocks[10].strText.push_back(wc);
    for (auto wc : std::string{ "0 / 0 6" })
        mvTextBlocks[11].strText.push_back(wc);
    for (auto wc : std::string{ "6" })
        mvTextBlocks[12].strText.push_back(wc);
}

void LobbyUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
    mvBitmapFileNames.clear();

}

void LobbyUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    UI::Initialize(device, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}