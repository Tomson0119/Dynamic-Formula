#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    //Text: 8 - MakeRoom, RoomNum[6], DenyMessage 
    //RoundRect: 17 - BigBackground, SmallBackground, SmallWhiteBackground, MakeRoom, RoomTitle[6], RoomBox[6], DenyBox
{
    SetTextCnt(8);
    SetRectCnt(17);
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
    SetTextCnt(8);
    SetRectCnt(17);
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
    Fonts.push_back(L"Fonts\\abberancy.ttf");

    FontLoad(Fonts);
}

RECT LobbyUI::MakeRect(float left, float top, float right, float bottom)
{
    return RECT(static_cast<LONG>(left), static_cast<LONG>(top), static_cast<LONG>(right), static_cast<LONG>(bottom));
}

int LobbyUI::OnProcessMouseClick(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc = MakeRect(GetTextBlock()[0].d2dLayoutRect.left, GetTextBlock()[0].d2dLayoutRect.top,
        GetTextBlock()[0].d2dLayoutRect.right, GetTextBlock()[0].d2dLayoutRect.bottom);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return 0;
    
    rc = MakeRect(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.40f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return mRoomNums[0];
    
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.40f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return mRoomNums[1];
    
    rc = MakeRect(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.58f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return mRoomNums[2];
    
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.58f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return mRoomNums[3];
    
    rc = MakeRect(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.76f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return mRoomNums[4];

    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.76f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN)
        return mRoomNums[5];
    
    return -1;
}

void LobbyUI::RoomMouseCheck(float dx, float dy, float left, float top, float right, float bottom, int index)
{
    RECT rc = MakeRect(left, top, right, bottom);
    if (MouseCollisionCheck(dx, dy, rc) /*&& !GetTextBlock()[index].strText.empty()*/)
    {
        SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 0.2f));
        SetIndexColor(11+index, D2D1::ColorF(D2D1::ColorF::Blue, 0.15f));
        SetIndexColor(17+index, D2D1::ColorF(D2D1::ColorF::Blue, 0.15f));
    }
    else
    {
        SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
        SetIndexColor(11 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
        SetIndexColor(17 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
    }
}

void LobbyUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    // MakeRoom
    RECT rc = MakeRect(GetTextBlock()[0].d2dLayoutRect.left, GetTextBlock()[0].d2dLayoutRect.top, 
        GetTextBlock()[0].d2dLayoutRect.right, GetTextBlock()[0].d2dLayoutRect.bottom);
    if (MouseCollisionCheck(dx, dy, rc))
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::DarkGray, 0.3f));
    else
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f));

    // Room 1, 2, 3, 4, 5, 6
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.25f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.40f, 1);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.40f, 2);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.25f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.58f, 3);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.58f, 4);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.25f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.76f, 5);
    RoomMouseCheck(dx, dy, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.76f, 6);

    // LeftArrow
    rc = MakeRect(GetFrameWidth() * 0.44f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.50f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc)) 
        aOpacities[2] = 0.2f;
    else
        aOpacities[2] = 0.7f;
    //Right Arrow
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.58f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc)) 
        aOpacities[3] = 0.2f;
    else
        aOpacities[3] = 0.7f;

    BuildSolidBrush(GetColors());
}

void LobbyUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc = MakeRect(GetFrameWidth() * 0.30f, GetFrameHeight() * 0.40f, GetFrameWidth() * 0.70f, GetFrameHeight() * 0.60f);
    if (mIsDenyBox)
        if (MouseCollisionCheck(dx, dy, rc))
        {
            mIsDenyBox = false;
        }
    BuildSolidBrush(GetColors());
}

void LobbyUI::Update(float GTime)
{
    if (mIsDenyBox)
    {
        SetIndexColor(7, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
        SetIndexColor(24, D2D1::ColorF(D2D1::ColorF::Black, 0.9f));
    }
    else
    {
        SetIndexColor(7, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
        SetIndexColor(24, D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    }
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
        {// DenyBox
        GetFrameWidth() * 0.30f,
        GetFrameHeight() * 0.40f,
        GetFrameWidth() * 0.70f,
        GetFrameHeight() * 0.60f
        },
        
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
    bool IsOutlined[17] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false };
    BeginDraw(nFrame);
    DrawBmp(LTRB, 0, 2, aOpacities);
    RectDraw(RectLTRB, FillLTRB, 0, GetRectCnt(), 0, IsOutlined);
    DrawBmp(LTRB, 2, 2, aOpacities);
    TextDraw(GetTextBlock());
    EndDraw(nFrame);
}

void LobbyUI::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'I':
            if (mIsDenyBox)
                mIsDenyBox = false;
            else                
                mIsDenyBox = true;
            break;
        case 'L':

            break;
        }
    }
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
    TextAlignments[7] = DWRITE_TEXT_ALIGNMENT_CENTER;

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
    GetTextBlock()[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.30f, GetFrameHeight() * 0.40f, GetFrameWidth() * 0.70f, GetFrameHeight() * 0.60f);
}

void LobbyUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    std::vector<D2D1::ColorF> colorList;
    /*Text*/
    // MakeRoom, RoomNum[6], DenyMessage
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));  
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));

    /*UI*/
    //BigBackGroundBox, SmallBackGroundBox, SmallWhiteBackGroundBox, MakeRoomBox,
    //RoomTitle[6], RoomBox[6], DenyBox
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
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.9f)); 
    SetColors(colorList);

    BuildSolidBrush(GetColors());

    //for (int i = 0; i < 6; ++i)
        //mRoomNums[i] = i+1;

    SetTextRect();

    GetTextBlock()[0].strText.assign("Make Room");
    //for(int i=0;i<6;++i)
        //GetTextBlock()[i+1].strText.assign(std::to_string(mRoomNums[i]));
}

void LobbyUI::UpdateRoomNums()
{
    for (int i = 0; i < 6; ++i)
    {
        if (mRoomNums[i] > 0)
            GetTextBlock()[i + 1].strText.assign(std::to_string(mRoomNums[i]));
        else
            GetTextBlock()[i + 1].strText.clear();
    }
}

void LobbyUI::UpdateDenyBoxText(const std::string& Msg)
{
    mDenyMessage = Msg;
    GetTextBlock()[7].strText.assign(mDenyMessage);
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