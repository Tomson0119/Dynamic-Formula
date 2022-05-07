#include "stdafx.h"
#include "RoomUI.h"
#include "NetLib/NetModule.h"

RoomUI::RoomUI(UINT nFrame, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, NetModule& netRef) 
    : UI(nFrame, device, pd3dCommandQueue), mNetRef(netRef)
    // Text: 19, RoundRect: 12
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8], AdminOrReady[8], StartFail
    //UI: NicknameBox[8], StartBox, CarSelectBox[2], MapSelectBox[2], StartFailBox
{
    SetTextCnt(20);
    SetRoundRectCnt(13);
    SetBitmapCnt(9);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

void RoomUI::SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue,
    ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height)
{
    Reset();

    UI::Initialize(device, pd3dCommandQueue);
    SetTextCnt(20);
    SetRoundRectCnt(13);
    SetBitmapCnt(9);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);

    BuildObjects(ppd3dRenderTargets, width, height);
}

void RoomUI::SetVectorSize(UINT nFrame)
{
    UI::SetVectorSize(nFrame);
  
    std::vector<std::wstring> BitmapFileNames;
    BitmapFileNames.push_back(L"Resources\\YellowBackGroundFlag.jpeg");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");
    BitmapFileNames.push_back(L"Resources\\CarSample1.png");

    SetBitmapFileNames(BitmapFileNames);
    //mvd2dLinearGradientBrush.resize(TextCnt);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Fonts\\Blazed.ttf");
    Fonts.push_back(L"Fonts\\Xenogears.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    for(size_t i=0;i<static_cast<size_t>(GetTextCnt())-3;++i)
        Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");

    FontLoad(Fonts);
}

void RoomUI::SetIndexInvisibleState(int index)
{
    SetIndexNicknameInvisible(index);
    SetIndexNotReady(index);
    SetIndexBackGroundInvisible(index);
    SetIndexCarInvisible(index);
    
    BuildSolidBrush(GetColors());
}

void RoomUI::SetIndexVisibleState(int index)
{
    SetIndexNicknameVisible(index);
    SetIndexBackGroundVisible(index);
    SetIndexCar(index);
}

void RoomUI::SetIndexReady(int index)
{
    SetIndexColor(11+index, D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
    BuildSolidBrush(GetColors());
}

void RoomUI::SetIndexNotReady(int index)
{
    SetIndexColor(11 + index, D2D1::ColorF(D2D1::ColorF::Beige, 0.0f));
    BuildSolidBrush(GetColors());
}

void RoomUI::SetIndexBackGroundVisible(int index)
{
    int bias = 21;
    switch (index)
    {
    case 0:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
        break;
    case 1:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));
        break;
    case 2:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Green, 1.0f));
        break;
    case 3:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Purple, 1.0f));
        break;
    case 4:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Black, 1.0f));
        break;
    case 5:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
        break;
    case 6:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Orange, 1.0f));
        break;
    case 7:
        SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f));
        break;
    }
    BuildSolidBrush(GetColors());
}

void RoomUI::SetIndexBackGroundInvisible(int index)
{
    int bias = 21;
    SetIndexColor(index + bias, D2D1::ColorF(D2D1::ColorF::Gray, 1.0f));
    BuildSolidBrush(GetColors());
}

bool RoomUI::MouseCollisionCheck(float x, float y, const TextBlock& TB)
{
    if (x<TB.d2dLayoutRect.right && x>TB.d2dLayoutRect.left &&
        y<TB.d2dLayoutRect.bottom && y>TB.d2dLayoutRect.top)
        return true;
    return false;
}

void RoomUI::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[0]))
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::FloralWhite, 0.4f));
    else
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::FloralWhite, 1.0f));

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1]))
        SetIndexColor(1, D2D1::ColorF(D2D1::ColorF::Beige, 0.4f));
    else
        SetIndexColor(1, D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[2]))
        SetIndexColor(2, D2D1::ColorF(D2D1::ColorF::Beige, 0.4f));
    else
        SetIndexColor(2, D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));

    BuildSolidBrush(GetColors());
}

void RoomUI::OnProcessMouseDown(WPARAM btnState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[19]))
        SetStateNotFail();
}

void RoomUI::SetStateFail(int result)
{
    //FailText설정
    GetTextBlock()[19].strText.assign("Fail");

    SetIndexColor(19, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    SetIndexColor(32, D2D1::ColorF(D2D1::ColorF::Black, 1.0f));
    BuildSolidBrush(GetColors());
}

void RoomUI::SetStateNotFail()
{
    SetIndexColor(19, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
    SetIndexColor(32, D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    BuildSolidBrush(GetColors());
}

void RoomUI::SetIndexIsAdmin(int index)
{
    for (int i = 0; i < 8; ++i)
    {
        if (i == index)
        {
            GetTextBlock()[i+static_cast<size_t>(11)].strText.assign("Admin");
            SetIndexColor(i + static_cast < size_t>(11), D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
        }
        else
        {
            GetTextBlock()[i+ static_cast < size_t>(11)].strText.assign("Ready");
            SetIndexColor(i + static_cast < size_t>(11), D2D1::ColorF(D2D1::ColorF::Beige, 0.0f));
        }
    }
    BuildSolidBrush(GetColors());
}

int RoomUI::OnProcessMouseClick(WPARAM btnState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);

    //레디 시작 버튼
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[0]) && WM_LBUTTONUP)
    {
        return 1;
    }

    //맵 변경 버튼
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[2]) && WM_LBUTTONUP)
        return -1;

    //나가기 버튼 == CarButton
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1]) && WM_LBUTTONUP)
        return -2;

    return 0;
}

void RoomUI::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'L':
            SetIndexIsAdmin(2);
            break;
        case VK_F11:
            SetStateFail(0);
            break;
        }
        break;
    }
}

void RoomUI::Update(float GTime)
{
    auto& mut = mNetRef.GetPlayerListMutex();
    mut.lock();
    const auto& playerList = mNetRef.GetPlayersInfo();
    mut.unlock();

    for (int i = 0; i < playerList.size(); ++i)
    {
        if (playerList[i].Empty)
        {
            SetIndexInvisibleState(i);
            continue;
        }
       
        GetTextBlock()[11 + static_cast<size_t>(i)].strText.assign("Ready");
        
        SetIndexIsAdmin(static_cast<int>(mNetRef.GetAdminIndex()));
        GetTextBlock()[3 + static_cast<size_t>(i)].strText.assign(playerList[i].Name);
        SetIndexVisibleState(i);

        if (mNetRef.IsAdmin())
        {
            GetTextBlock()[0].strText.assign("Start");
        }
        else
            GetTextBlock()[0].strText.assign("Ready");
       
        if (mNetRef.IsAdmin())
            SetIndexColor(11 + i, D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));

        if (playerList[i].Ready)
            SetIndexReady(i);
        else
            SetIndexNotReady(i);
    }
}

void RoomUI::Draw(UINT nFrame)
{
    XMFLOAT4 RectLTRB[] =
    {
        {
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.265f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.495f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.505f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.735f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.625f
        },
        {
        GetFrameWidth() * 0.265f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.495f,
        GetFrameHeight() * 0.625f
        },
        {
        GetFrameWidth() * 0.505f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.735f,
        GetFrameHeight() * 0.625f
        },
        {
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.625f
        },
        {//StartOrReady
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.65f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.8f
        },
        {// CarSelect
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.65f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.8f
        },
        {// MapSelect
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.825f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.975f
        }
    };
    XMFLOAT4 FillLTRB[] =
    {
        {//BackGround
        GetFrameWidth() * 0.015f,
        GetFrameHeight() * 0.015f,
        GetFrameWidth() * 0.985f,
        GetFrameHeight() * 0.985f
        },
        {//Car[0]
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.32f
        },
        {//Car[1]
        GetFrameWidth() * 0.265f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.495f,
        GetFrameHeight() * 0.32f
        },
        {//Car[2]
        GetFrameWidth() * 0.505f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.735f,
        GetFrameHeight() * 0.32f
        },
        {//Car[3]
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.32f
        },
        {//Car[4]
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.625f
        },
        {//Car[5]
        GetFrameWidth() * 0.265f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.495f,
        GetFrameHeight() * 0.625f
        },
        {//Car[6]
        GetFrameWidth() * 0.505f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.735f,
        GetFrameHeight() * 0.625f
        },
        {//Car[7]
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.625f
        },
        {//StartOrReady
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.65f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.8f
        },
        {// CarSelect
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.65f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.8f
        },
        {// MapSelect
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.825f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.975f
        },
        {// Fail
        GetFrameWidth() * 0.4f,
        GetFrameHeight() * 0.4f,
        GetFrameWidth() * 0.6f,
        GetFrameHeight() * 0.6f
        }
    };
    XMFLOAT4 LTRB[] =
    {
        {
        0.0f,
        0.0f,
        GetFrameWidth(),
        GetFrameHeight()
        },
        {
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.265f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.495f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.505f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.735f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.025f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.32f
        },
        {
        GetFrameWidth() * 0.025f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.255f,
        GetFrameHeight() * 0.625f
        },
        {
        GetFrameWidth() * 0.265f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.495f,
        GetFrameHeight() * 0.625f
        },
        {
        GetFrameWidth() * 0.505f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.735f,
        GetFrameHeight() * 0.625f
        },
        {
        GetFrameWidth() * 0.745f,
        GetFrameHeight() * 0.33f,
        GetFrameWidth() * 0.975f,
        GetFrameHeight() * 0.625f
        }
    };
    bool IsOutlined[13] = { false,false,false,false,false,false,false,false,false,false,false,false, false };

    BeginDraw(nFrame); 
    DrawBmp(LTRB, 0, 1, BitmapOpacities);
    RoundedRectDraw(RectLTRB, FillLTRB, 0, IsOutlined);
    DrawBmp(LTRB, 1, 8, BitmapOpacities);
    TextDraw(GetTextBlock());
    EndDraw(nFrame);
}

void RoomUI::CreateFontFormat()
{
    std::vector<float> fFontSize;
    fFontSize.push_back(GetFrameHeight() * 0.065f);  //Start
    fFontSize.push_back(GetFrameHeight() * 0.07f); //CarSelect
    fFontSize.push_back(GetFrameHeight() * 0.07f); //MapSelect
    for(size_t i=0;i<static_cast<size_t>(GetTextCnt())-3;++i)
        fFontSize.push_back(GetFrameHeight() * 0.05f);
    SetFontSize(fFontSize);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Blazed");
    Fonts.push_back(L"Xenogears");
    Fonts.push_back(L"abberancy");
    for(size_t i=0;i<static_cast<size_t>(GetTextCnt())-3;++i)
        Fonts.push_back(L"abberancy");
    Fonts.push_back(L"abberancy");

    SetFonts(Fonts);

    std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
    TextAlignments.resize(GetTextCnt());
    TextAlignments[0] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
    for(size_t i=3;i<static_cast<size_t>(GetTextCnt());++i)
        TextAlignments[i] = DWRITE_TEXT_ALIGNMENT_CENTER;

    SetTextAllignments(TextAlignments);

    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}


void RoomUI::SetTextRect()
{//Text: StartOrReady, CarSelect, MapSelect, Nickname[8], RedayOrStart[8]
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.735f, GetFrameHeight() * 0.65f, GetFrameWidth() * 0.975f, GetFrameHeight() * 0.8f);
    GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.65f, GetFrameWidth() * 0.255f, GetFrameHeight() * 0.8f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.825f, GetFrameWidth() * 0.23f, GetFrameHeight() * 0.975f);
    
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.075f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.075f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.075f);
    GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.075f);
    GetTextBlock()[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.38f);
    GetTextBlock()[8].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.38f);
    GetTextBlock()[9].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.38f);
    GetTextBlock()[10].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.38f);
    
    GetTextBlock()[11].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.265f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.32f);
    GetTextBlock()[12].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.265f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.32f);
    GetTextBlock()[13].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.265f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.32f);
    GetTextBlock()[14].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.265f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.32f);
    GetTextBlock()[15].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.575f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.625f);
    GetTextBlock()[16].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.575f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.625f);
    GetTextBlock()[17].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.575f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.625f);
    GetTextBlock()[18].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.575f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.625f);

    GetTextBlock()[19].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.4f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.6f);
}

void RoomUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    
    std::vector<D2D1::ColorF> colorList;
    /*Text*/
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8], RedayOrStart[8] - 19
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::FloralWhite, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
    for (int i = 0; i < 8;++i) // Nckname[8]
        colorList.push_back(D2D1::ColorF(D2D1::ColorF::LightGray, 1.0f));
    for (int i = 0; i < 8; ++i) // StartOrReady[8]
        colorList.push_back(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    /*UI*/
    //UI:  BackGround, NickNameBox[8], StartBox, CarBox, MapBox - 12
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gold , 0.9f));

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Green, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Purple, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Orange, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f));

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Green, 1.0f));

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 1.0f));


    SetColors(colorList);

    BuildSolidBrush(GetColors());

    SetTextRect(); 

    GetTextBlock()[1].strText.assign("Out");
    GetTextBlock()[2].strText.assign("Map");

    SetStateNotFail();

    GetTextBlock()[19].strText.assign("Fail");
}

void RoomUI::Reset()
{
    UI::Reset();
}

void RoomUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    UI::Initialize(device, pd3dCommandQueue);
    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
    BuildObjects(ppd3dRenderTargets, width, height);
}