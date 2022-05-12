#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    //Text: 7, RoundRect: 18
{
    SetTextCnt(7);
    SetRectCnt(18);
    SetBitmapCnt(4);
    //SetGradientCnt(12);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0;i<static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

void LobbyUI::SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue,
    ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height)
{
    Reset();

    UI::Initialize(device, pd3dCommandQueue);
    SetTextCnt(7);
    SetRectCnt(18);
    SetBitmapCnt(4);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);

    BuildObjects(ppd3dRenderTargets, width, height);
}

bool LobbyUI::MouseCollisionCheck(float x, float y, const RECT& rc)
{
    if (x< rc.right&& x>rc.left &&
        y<rc.bottom && y>rc.top)
        return true;
    return false;
}

void LobbyUI::SetVectorSize(UINT nFrame)
{
    UI::SetVectorSize(nFrame);

    std::vector<std::wstring> BitmapFileNames;
    BitmapFileNames.push_back(L"Resources\\SampleImg.jpg");
    BitmapFileNames.push_back(L"Resources\\YellowBackGroundFlag.jpeg");
    BitmapFileNames.push_back(L"Resources\\LeftArrow.png");
    BitmapFileNames.push_back(L"Resources\\RightArrow.png");


    SetBitmapFileNames(BitmapFileNames);
    //mvd2dLinearGradientBrush.resize(TextCnt);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Fonts\\Blazed.ttf");
    Fonts.push_back(L"Fonts\\Xenogears.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");

    FontLoad(Fonts);
}

RECT LobbyUI::MakeRect(float left, float top, float right, float bottom)
{
    return RECT(left, top, right, bottom);
}

int LobbyUI::OnProcessMouseClick(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc = MakeRect(GetTextBlock()[0].d2dLayoutRect.left, GetTextBlock()[0].d2dLayoutRect.top,
        GetTextBlock()[0].d2dLayoutRect.right, GetTextBlock()[0].d2dLayoutRect.bottom);
    if (MouseCollisionCheck(dx, dy, rc)
        && buttonState & WM_LBUTTONDOWN)
    {
        return 1;
    }
    /*rc = MakeRect(GetTextBlock()[1].d2dLayoutRect.left, GetTextBlock()[1].d2dLayoutRect.top,
        GetTextBlock()[1].d2dLayoutRect.right, GetTextBlock()[1].d2dLayoutRect.bottom);
    if ( (MouseCollisionCheck(dx, dy, rc)
        || MouseCollisionCheck(dx, dy, rc))
        && buttonState & WM_LBUTTONUP )
    {

        return 2;
    }*/
    
    return 0;
}

void LobbyUI::RoomMouseCheck(float dx, float dy, float left, float top, float right, float bottom, int index)
{
    RECT rc = MakeRect(left, top, right, bottom);
    if (MouseCollisionCheck(dx, dy, rc))
    {
        SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 0.2f));
        SetIndexColor(10+index, D2D1::ColorF(D2D1::ColorF::Blue, 0.15f));
        SetIndexColor(16+index, D2D1::ColorF(D2D1::ColorF::Blue, 0.15f));
    }
    else
    {
        SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
        SetIndexColor(10 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
        SetIndexColor(16 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
    }
}

void LobbyUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc = MakeRect(GetTextBlock()[0].d2dLayoutRect.left, GetTextBlock()[0].d2dLayoutRect.top, 
        GetTextBlock()[0].d2dLayoutRect.right, GetTextBlock()[0].d2dLayoutRect.bottom);
    if (MouseCollisionCheck(dx, dy, rc))
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::DarkGray, 0.3f));
    else
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f));

    // 1, 2, 3, 4, 5, 6
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.25f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.40f, 1);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.40f, 2);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.25f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.58f, 3);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.58f, 4);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.25f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.76f, 5);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.76f, 6);

    

    rc = MakeRect(GetFrameWidth() * 0.44f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.50f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc)) // LeftArrow
        aOpacities[2] = 0.2f;
    else
        aOpacities[2] = 0.7f;

    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.58f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc)) // RightArrow
        aOpacities[3] = 0.2f;
    else
        aOpacities[3] = 0.7f;

    BuildSolidBrush(GetColors());
}

void LobbyUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc;
    for (int i = 1; i < static_cast<int>(GetTextCnt()); ++i)
    {
        /*rc = MakeRect(GetTextBlock()[i].d2dLayoutRect.left, GetTextBlock()[i].d2dLayoutRect.top,
            GetTextBlock()[i].d2dLayoutRect.right, GetTextBlock()[i].d2dLayoutRect.bottom);
        if (MouseCollisionCheck(dx, dy, rc) && buttonState==WM_LBUTTONDOWN)
        {
            SetIndexColor(i/2, D2D1::ColorF(D2D1::ColorF::White, 0.2f));
        }
        else SetIndexColor(i / 2, D2D1::ColorF(D2D1::ColorF::White, 1.0f));*/
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
        GetFrameHeight() * 0.245f,
        GetFrameWidth() * 0.48f,
        GetFrameHeight() * 0.3f
        },
        {//RoomTitle2
            GetFrameWidth() * 0.54f,
            GetFrameHeight() * 0.245f,
            GetFrameWidth() * 0.75f,
            GetFrameHeight() * 0.3f
        },
        {//RoomTitle3
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.425f,
            GetFrameWidth() * 0.48f,
            GetFrameHeight() * 0.48f
        },
        {//RoomTitle4
        GetFrameWidth() * 0.54f,
        GetFrameHeight() * 0.425f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() * 0.48f
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
        GetFrameHeight() * 0.23f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.40f
        },
        {//RoomBox2
            GetFrameWidth() * 0.52f,
            GetFrameHeight() * 0.23f,
            GetFrameWidth() * 0.77f,
            GetFrameHeight() * 0.40f
        },
        {//RoomBox3
            GetFrameWidth() * 0.25f,
            GetFrameHeight() * 0.41f,
            GetFrameWidth() * 0.5f,
            GetFrameHeight() * 0.58f
        },
        {//RoomBox4
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.41f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.58f
        },
        {//RoomBox5
        GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.59f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.76f
        },
        {//RoomBox6
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.59f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.76f
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
        GetFrameHeight() * 0.245f,
        GetFrameWidth() * 0.48f,
        GetFrameHeight() * 0.3f
        },
        {//RoomTitle2
            GetFrameWidth() * 0.54f,
            GetFrameHeight() * 0.245f,
            GetFrameWidth() * 0.75f,
            GetFrameHeight() * 0.3f
        },
        {//RoomTitle3
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.425f,
            GetFrameWidth() * 0.48f,
            GetFrameHeight() * 0.48f
        },
        {//RoomTitle4
        GetFrameWidth() * 0.54f,
        GetFrameHeight() * 0.425f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() *0.48f
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
        GetFrameHeight() * 0.23f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.40f
        },
        {//RoomBox2
            GetFrameWidth() * 0.52f,
            GetFrameHeight() * 0.23f,
            GetFrameWidth() * 0.77f,
            GetFrameHeight() * 0.40f
        },
        {//RoomBox3
            GetFrameWidth() * 0.25f,
            GetFrameHeight() * 0.41f,
            GetFrameWidth() * 0.5f,
            GetFrameHeight() * 0.58f
        },
        {//RoomBox4
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.41f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.58f
        },
        {//RoomBox5
        GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.59f,
        GetFrameWidth() * 0.5f,
        GetFrameHeight() * 0.76f
        },
        {//RoomBox6
        GetFrameWidth() * 0.52f,
        GetFrameHeight() * 0.59f,
        GetFrameWidth() * 0.77f,
        GetFrameHeight() * 0.76f
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
            GetFrameWidth() * 0.50f,
            GetFrameHeight() * 0.82f
        },
        {
             GetFrameWidth() * 0.52f,
           GetFrameHeight() * 0.76f,
            GetFrameWidth() * 0.58f,
            GetFrameHeight() * 0.82f
        }
    };
    bool IsOutlined[18] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false };
    BeginDraw(nFrame);
    DrawBmp(LTRB, 0, 2, aOpacities);
    RectDraw(RectLTRB, FillLTRB, 0, GetRectCnt(), 0, IsOutlined);
    DrawBmp(LTRB, 2, 2, aOpacities);
    TextDraw(GetTextBlock());
    EndDraw(nFrame);
}

void LobbyUI::CreateFontFormat()
{
    std::vector<float>fFontSize;
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    SetFontSize(fFontSize);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Blazed");
    Fonts.push_back(L"Xenogears");
    Fonts.push_back(L"abberancy");
    Fonts.push_back(L"abberancy");
    Fonts.push_back(L"abberancy");
    Fonts.push_back(L"abberancy");
    Fonts.push_back(L"abberancy");
    SetFonts(Fonts);

    std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
    TextAlignments.resize(GetTextCnt());
    TextAlignments[0] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_CENTER;
    SetTextAllignments(TextAlignments);

    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}

void LobbyUI::SetTextRect()
{//MakeRoom, Room1, Room2, Room3, Room4, Room5, Room6, Box[6], LeftArrowBox, RightArrowBox
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.28125f, GetFrameHeight() * 0.055f, GetFrameWidth() * 0.46875f, GetFrameHeight() * 0.11f);
    GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.245f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.3f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.245f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.3f);
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.425f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.48f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.425f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.48f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.605f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.66f);
    GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.605f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.66f);
}

void LobbyUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    std::vector<D2D1::ColorF> colorList;
    /*Text*/
    // MakeRoom, RoomNum[6]
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));  
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    /*UI*/
    //BigBackGroundBox, SmallBackGroundBox, SmallWhiteBackGroundBox, MakeRoomBox,
    //RoomTitle[6], RoomBox[6], Leftarrow, Rightarrow
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.98f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 0.3f)); //22
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::LightGray, 0.3f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::LightGray, 0.3f)); 
    SetColors(colorList);

    BuildSolidBrush(GetColors());

    for (int i = 0; i < 6; ++i)
        mRoomNums[i] = i+1;

    SetTextRect();

    for (auto &wc : std::string{ "MakeRoom" })
        GetTextBlock()[0].strText.push_back(wc);
    for (auto &wc : std::string{std::to_string(mRoomNums[0]) })
        GetTextBlock()[1].strText.push_back(wc);
    for (auto &wc : std::string{ std::to_string(mRoomNums[1]) })
        GetTextBlock()[2].strText.push_back(wc);
    for (auto &wc : std::string{ std::to_string(mRoomNums[2]) })
        GetTextBlock()[3].strText.push_back(wc);
    for (auto &wc : std::string{ std::to_string(mRoomNums[3]) })
        GetTextBlock()[4].strText.push_back(wc);
    for (auto &wc : std::string{ std::to_string(mRoomNums[4]) })
        GetTextBlock()[5].strText.push_back(wc);
    for (auto &wc : std::string{ std::to_string(mRoomNums[5]) })
        GetTextBlock()[6].strText.push_back(wc);
}

void LobbyUI::UpdateRoomNumsText()
{
    for (UINT i = 1; i < GetTextCnt(); ++i)
    {
        GetTextBlock()[i].strText.clear();
        for (auto& str : std::string{ std::to_string(mRoomNums[i - 1]) })
        {
            GetTextBlock()[i].strText.push_back(str);
        }
    }
}

void LobbyUI::Reset()
{
    UI::Reset();
}

void LobbyUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    UI::Initialize(device, pd3dCommandQueue);
    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
    BuildObjects(ppd3dRenderTargets, width, height);
}