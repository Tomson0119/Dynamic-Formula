#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    //Text: 13, RoundRect: 18
{
    SetTextCnt(13);
    SetRoundRectCnt(18);
    SetVectorSize(nFrame, GetTextCnt());
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
    ResizeFontSize(TextCnt);
    ResizeFonts(TextCnt);

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
    for (int i = 1; i < static_cast<int>(GetTextCnt()); ++i)
    {// 12, 34, 56, 78, 910, 1112
        if (MouseCollisionCheck(dx, dy, mvTextBlocks[i]))
        {
            mvColors[(((i+1)/2)*2)-1].a = 0.1f;
        }
        else mvColors[i].a = 0.9f;
    }
    UI::BuildSolidBrush(GetRoundRectCnt() + 1, GetTextCnt(), mvColors);

}

void LobbyUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    for (int i = 1; i < static_cast<int>(GetTextCnt()); ++i)
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
            GetFrameWidth() * 0.04f,
            GetFrameHeight() * 0.02f,
            GetFrameWidth() * 0.96f,
            GetFrameHeight() * 0.96f
        },
        {//SmallBackGroundBox
            GetFrameWidth() * 0.18f,
            GetFrameHeight() * 0.04f,
            GetFrameWidth() * 0.86f,
            GetFrameHeight() * 0.86f
        },
        {//SmallWhiteBackGroundBox
        GetFrameWidth() * 0.20f,
        GetFrameHeight() * 0.18f,
        GetFrameWidth() * 0.80f,
        GetFrameHeight() * 0.84f
        },
        {//MakeRoomBox
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.055f,
            GetFrameWidth() * 0.48f,
            GetFrameHeight() * 0.11f
        },
        {//RoomTitle1
        GetFrameWidth() * 0.27f,
        GetFrameHeight() * 0.26f,
        GetFrameWidth() * 0.48f,
        GetFrameHeight() * 0.33f
        },
        {//RoomTitle2
            GetFrameWidth() * 0.54f,
            GetFrameHeight() * 0.26f,
            GetFrameWidth() * 0.75f,
            GetFrameHeight() * 0.33f
        },
        {//RoomTitle3
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.44f,
            GetFrameWidth() * 0.48f,
            GetFrameHeight() * 0.5f
        },
        {//RoomTitle4
        GetFrameWidth() * 0.54f,
        GetFrameHeight() * 0.44f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() * 0.5f
        },
        {//RoomTitle5
        GetFrameWidth() * 0.27f,
        GetFrameHeight() * 0.605f,
        GetFrameWidth() * 0.48f,
        GetFrameHeight() * 0.66f
        },
        {//RoomTitle6
        GetFrameWidth() * 0.54f,
        GetFrameHeight() * 0.605f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() * 0.66f
        },
        {//RoomBox1
        GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.25f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.42f
        },
        {//RoomBox2
            GetFrameWidth() * 0.52f,
            GetFrameHeight() * 0.25f,
            GetFrameWidth() * 0.77f,
            GetFrameHeight() * 0.42f
        },
        {//RoomBox3
            GetFrameWidth() * 0.25f,
            GetFrameHeight() * 0.42f,
            GetFrameWidth() * 0.5f,
            GetFrameHeight() * 0.57f
        },
        {//RoomBox4
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.42f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.57f
        },
        {//RoomBox5
        GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.57f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.75f
        },
        {//RoomBox6
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.57f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.75f
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {//BigBackGroundBox
            GetFrameWidth() * 0.04f,
            GetFrameHeight() * 0.02f,
            GetFrameWidth() * 0.96f,
            GetFrameHeight() * 0.96f
        },
        {//SmallBackGroundBox
            GetFrameWidth() * 0.18f,
            GetFrameHeight() * 0.04f,
            GetFrameWidth() * 0.86f,
            GetFrameHeight() * 0.86f
        },
        {//SmallWhiteBackGroundBox
        GetFrameWidth() * 0.20f,
        GetFrameHeight() * 0.18f,
        GetFrameWidth() * 0.80f,
        GetFrameHeight() * 0.84f
        },
        {//MakeRoomBox
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.055f,
            GetFrameWidth() * 0.48f,
            GetFrameHeight() * 0.11f
        },
        {//RoomTitle1
        GetFrameWidth() * 0.27f,
        GetFrameHeight() * 0.26f,
        GetFrameWidth() * 0.48f,
        GetFrameHeight() * 0.33f
        },
        {//RoomTitle2
            GetFrameWidth() * 0.54f,
            GetFrameHeight() * 0.26f,
            GetFrameWidth() * 0.75f,
            GetFrameHeight() * 0.33f
        },
        {//RoomTitle3
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.44f,
            GetFrameWidth() * 0.48f,
            GetFrameHeight() * 0.5f
        },
        {//RoomTitle4
        GetFrameWidth() * 0.54f,
        GetFrameHeight() * 0.44f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() *0.5f
        },
        {//RoomTitle5
        GetFrameWidth() * 0.27f,
        GetFrameHeight() * 0.605f,
        GetFrameWidth() * 0.48f,
        GetFrameHeight() * 0.66f
        },
        {//RoomTitle6
        GetFrameWidth() * 0.54f,
        GetFrameHeight() * 0.605f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() * 0.66f
        },
        {//RoomBox1
        GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.25f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.42f
        },
        {//RoomBox2
            GetFrameWidth() * 0.52f,
            GetFrameHeight() * 0.25f,
            GetFrameWidth() * 0.77f,
            GetFrameHeight() * 0.42f
        },
        {//RoomBox3
            GetFrameWidth() * 0.25f,
            GetFrameHeight() * 0.42f,
            GetFrameWidth() * 0.5f,
            GetFrameHeight() * 0.57f
        },
        {//RoomBox4
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.42f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.57f
        },
        {//RoomBox5
        GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.57f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.75f
        },
        {//RoomBox6
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.57f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.75f
        },
        {//LeftArrowBox
        GetFrameWidth() * 0.044f,
        GetFrameHeight() * 0.076f,
        GetFrameWidth() * 0.05f,
        GetFrameHeight() * 0.082f
        },
        {//RightArrowBox
        GetFrameWidth() * 0.052f,
        GetFrameHeight() * 0.076f,
        GetFrameWidth() * 0.058f,
        GetFrameHeight() * 0.082f
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
            GetFrameWidth(),
            GetFrameHeight()
        },
        {
             GetFrameWidth() * 0.44f,
           GetFrameHeight() * 0.76f,
            GetFrameWidth() * 0.58f,
            GetFrameHeight() * 0.82f
        }
    };
    bool IsOutlined[18] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false };
    float aOpacities[3] = { 0.5f, 1.0f, 0.5f };
    UI::BeginDraw(nFrame);
    UI::DrawBmp(LTRB, 0, 2, aOpacities);
    UI::RoundedRectDraw(RectLTRB, FillLTRB, GetTextCnt() + 1, 0, 0, IsOutlined);
    UI::DrawBmp(LTRB, 2, 1, aOpacities);
    UI::TextDraw(nFrame, GetTextCnt(), mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LobbyUI::CreateFontFormat()
{
    std::vector<float>fFontSize;
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
    fFontSize.push_back(GetFrameHeight() / 15.0f);
    fFontSize.push_back(GetFrameHeight() / 15.0f);

    SetFontSize(fFontSize);

    std::vector<std::wstring> Fonts;
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

    SetFonts(Fonts);

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

    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextCnt(), TextAlignments);
}

void LobbyUI::SetTextRect()
{//MakeRoom, Room1, Room2, Room3, Room4, Room5, Room6
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.28125f, GetFrameHeight() * 0.055f, GetFrameWidth() * 0.46875f, GetFrameHeight() * 0.11f);

    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.26f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.33f);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.25f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.42f);

    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.26f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.33f);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.25f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.42f);

    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.44f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.5f);
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.42f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.57f);

    mvTextBlocks[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.44f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.5f);
    mvTextBlocks[8].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.42f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.57f);

    mvTextBlocks[9].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.605f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.66f);
    mvTextBlocks[10].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.57f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.75f);

    mvTextBlocks[11].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.605f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.66f);
    mvTextBlocks[12].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.57f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.75f);
}

void LobbyUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    //SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    
    D2D1::ColorF colorList[31] = { D2D1::ColorF(D2D1::ColorF::DarkGray , 1.0f), D2D1::ColorF(D2D1::ColorF::DarkGray , 1.0f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), 
        D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), 
        D2D1::ColorF(D2D1::ColorF::Gray, 0.98f), D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f),D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::White, 0.9f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f),
        D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f), D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::Blue, 0.3f) , D2D1::ColorF(D2D1::ColorF::LightGray, 0.3f) , D2D1::ColorF(D2D1::ColorF::LightGray, 0.3f)};
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    for (auto color : colorList)
        mvColors.push_back(color);
    UI::BuildSolidBrush(GetRoundRectCnt() + 1, GetTextCnt(), mvColors);

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
    SetVectorSize(nFrame, GetTextCnt());
    UI::Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < mvBitmapFileNames.size(); ++i)
        LoadBitmapResourceFromFile(mvBitmapFileNames[i], i);
    PreDraw(ppd3dRenderTargets, width, height);
}