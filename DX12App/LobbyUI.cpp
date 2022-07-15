#include "stdafx.h"
#include "LobbyUI.h"

LobbyUI::LobbyUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    //Text: 26 - MakeRoom, RoomNum[6], DenyMessage, PlayerCount[6], GameStarted[6], MapID[6] 
    //RoundRect: 17 - BigBackground, SmallBackground, SmallWhiteBackground, MakeRoom, RoomTitle[6], RoomBox[6], DenyBox
{
    SetTextCnt(26);
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
    SetTextCnt(26);
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
    for(int i=0;i<26-2;++i)
    Fonts.push_back(L"Fonts\\abberancy.ttf");

    FontLoad(Fonts);
}

RECT LobbyUI::MakeRect(float left, float top, float right, float bottom)
{
    return RECT(static_cast<LONG>(left), static_cast<LONG>(top), static_cast<LONG>(right), static_cast<LONG>(bottom));
}

void LobbyUI::RoomEmptyProcess()
{
    for (int i = 1; i < 7; ++i) 
    {
        if (mIsOpened[i-1]) //Closed
        {
            if (mIsGameStarted[i - 1])
            {
                SetIndexColor(i, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
                SetIndexColor(GetTextCnt() + 3 + i, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
                SetIndexColor(GetTextCnt() + 9 + i, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
            }
            else
            {
                SetIndexColor(i, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
                SetIndexColor(GetTextCnt() + 3 + i, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
                SetIndexColor(GetTextCnt() + 9 + i, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
            }
        }
        else 
        {
            SetIndexColor(i, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
            SetIndexColor(GetTextCnt() + 3 + i, D2D1::ColorF(D2D1::ColorF::Gray, 0.7f));
            SetIndexColor(GetTextCnt() + 9 + i, D2D1::ColorF(D2D1::ColorF::Gray, 0.7f));
        }
    }
    //BuildSolidBrush(GetColors());
}

int LobbyUI::OnProcessMouseClick(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc = MakeRect(GetTextBlock()[0].d2dLayoutRect.left, GetTextBlock()[0].d2dLayoutRect.top,
        GetTextBlock()[0].d2dLayoutRect.right, GetTextBlock()[0].d2dLayoutRect.bottom);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN) // make room
        return -2;
    rc = MakeRect(GetFrameWidth() * 0.44f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.50f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc) && buttonState & WM_LBUTTONDOWN) // Left Arrow
        return -3;
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.58f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc) && buttonState & WM_LBUTTONDOWN) // Right Arrow
        return -4;
    
    rc = MakeRect(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.40f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN && !GetTextBlock()[1].strText.empty()) // room 1
        return mRoomNums[0];
    
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.23f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.40f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN && !GetTextBlock()[2].strText.empty()) // room 2
        return mRoomNums[1];
    
    rc = MakeRect(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.58f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN && !GetTextBlock()[3].strText.empty()) // room 3
        return mRoomNums[2];
    
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.41f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.58f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN && !GetTextBlock()[4].strText.empty()) // room 4
        return mRoomNums[3];
    
    rc = MakeRect(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.76f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN && !GetTextBlock()[5].strText.empty()) // room 5
        return mRoomNums[4];

    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.59f, GetFrameWidth() * 0.77f, GetFrameHeight() * 0.76f);
    if (MouseCollisionCheck(dx, dy, rc)&& buttonState & WM_LBUTTONDOWN && !GetTextBlock()[6].strText.empty()) // room 6
        return mRoomNums[5];
    
    return -1;
}

void LobbyUI::SetRoomInfo(int index, int RoomID, unsigned char PlayerCount, unsigned char MapID, bool GameStarted, bool Opened)
{
    mRoomNums[index] = RoomID;
    //mPlayerCount[index] = PlayerCount;
    //mMapID[index] = MapID;
    mIsGameStarted[index] = GameStarted;
    mIsOpened[index] = Opened;
}

void LobbyUI::RoomMouseCheck(float dx, float dy, float left, float top, float right, float bottom, int index)
{
    RECT rc = MakeRect(left, top, right, bottom);
    auto& sound = GetSound();
    if (MouseCollisionCheck(dx, dy, rc) && !mIsOpened[index-1])
    {
        if (!mIsGameStarted[index-1]) // æ» Ω√¿€
        {
            mIsMouseCollisionRoom[index] = false;
            SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
            SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Gray, 0.7f));
            SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Gray, 0.7f));
        }
        else 
        {
            mIsMouseCollisionRoom[index] = false;
            SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
            SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
            SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
        }
    }
    else if(!MouseCollisionCheck(dx, dy, rc) && !mIsOpened[index-1])
    {
        if (!mIsGameStarted[index-1] )
        {
            mIsMouseCollisionRoom[index] = false;
            SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
            SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Gray, 0.7f));
            SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Gray, 0.7f));
        }
        else
        {
            mIsMouseCollisionRoom[index] = false;
            SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 0.2f));
            SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
            SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
        }
    }
    else if (mIsOpened[index - 1])
    {
        if (mIsGameStarted[index - 1])
        {
            mIsMouseCollisionRoom[index] = false;
            SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
            SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
            SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Green, 0.7f));
        }
        else
        {
            if (MouseCollisionCheck(dx, dy, rc)) 
            {
                if (!mIsMouseCollisionRoom[index])
                {
                    sound.Play(NORMAL_VOLUME, static_cast<int>(LOBBYUI_SOUND_TRACK::MOUSE_COLLISION));
                    mIsMouseCollisionRoom[index] = true;
                }
                SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 0.2f));
                SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.15f));
                SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.15f));
            }
            else
            {
                mIsMouseCollisionRoom[index] = false;
                SetIndexColor(index, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
                SetIndexColor(GetTextCnt() + 3 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
                SetIndexColor(GetTextCnt() + 9 + index, D2D1::ColorF(D2D1::ColorF::Blue, 0.3f));
            }
        }
    }

}

void LobbyUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    auto& sound = GetSound();
    // MakeRoom
    RECT rc = MakeRect(GetTextBlock()[0].d2dLayoutRect.left, GetTextBlock()[0].d2dLayoutRect.top, 
        GetTextBlock()[0].d2dLayoutRect.right, GetTextBlock()[0].d2dLayoutRect.bottom);
    if (MouseCollisionCheck(dx, dy, rc))
    {
        if (!mIsMouseCollisionMakeRect)
        {
            mIsMouseCollisionMakeRect = true;
            sound.Play(NORMAL_VOLUME, static_cast<int>(LOBBYUI_SOUND_TRACK::MOUSE_COLLISION));
        }
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::DarkGray, 0.3f));
    }
    else
    {
        mIsMouseCollisionMakeRect = false;
        SetIndexColor(0, D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f));
    }

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
    {
        if (!mIsMouseCollisionLeftArrow)
        {
            mIsMouseCollisionLeftArrow = true;
            sound.Play(NORMAL_VOLUME, static_cast<int>(LOBBYUI_SOUND_TRACK::MOUSE_COLLISION));
        }
        aOpacities[2] = 0.2f;
    }
    else
    {
        mIsMouseCollisionLeftArrow = false;
        aOpacities[2] = 0.7f;
    }
    //Right Arrow
    rc = MakeRect(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.76f, GetFrameWidth() * 0.58f, GetFrameHeight() * 0.82f);
    if (MouseCollisionCheck(dx, dy, rc))
    {
        if (!mIsMouseCollisionRightArrow)
        {
            mIsMouseCollisionRightArrow = true;
            sound.Play(NORMAL_VOLUME, static_cast<int>(LOBBYUI_SOUND_TRACK::MOUSE_COLLISION));
        }
        aOpacities[3] = 0.2f;
    }
    else
    {
        mIsMouseCollisionRightArrow = false;
        aOpacities[3] = 0.7f;
    }

    BuildSolidBrush(GetColors());
}

void LobbyUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    RECT rc = MakeRect(GetFrameWidth() * 0.30f, GetFrameHeight() * 0.40f, GetFrameWidth() * 0.70f, GetFrameHeight() * 0.60f);
    if (mIsDenyBox)
    {
        if (MouseCollisionCheck(dx, dy, rc))
        {
            SetInvisibleDenyBox();
        }
    }
    
}

void LobbyUI::SetVisibleDenyBox()
{
    SetIndexColor(7, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    SetIndexColor(42, D2D1::ColorF(D2D1::ColorF::Black, 0.9f));
    UpdateDenyBoxText();
    BuildSolidBrush(GetColors());
}

void LobbyUI::SetInvisibleDenyBox()
{
    SetIndexColor(7, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
    SetIndexColor(42, D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    BuildSolidBrush(GetColors());
}

void LobbyUI::Update(float GTime)
{
    BuildSolidBrush(GetColors());
}

void LobbyUI::UpdateRoomIDTextsIndex(int index, int RoomID, bool Opened)
{
    if (mIsOpened[index])
    {
        mRoomNums[index] = RoomID;
        GetTextBlock()[index + 1].strText.assign(std::to_string(RoomID + 1));
    }
    else
    {
        mRoomNums[index] = -1;
        GetTextBlock()[index + 1].strText.clear();
    }
}


void LobbyUI::UpdatePlayerCountTextsIndex(int index, int PlayerCount)
{
    if (PlayerCount > 0)
        GetTextBlock()[index + 8].strText.assign(std::to_string(PlayerCount) + " / 8");
    else
        GetTextBlock()[index + 8].strText.clear();
}

void LobbyUI::UpdateMapIDTextsIndex(int index, int MapID)
{
    //
    if (mIsOpened[index]) {
        if (MapID)
            GetTextBlock()[index + 14].strText.assign("");
        else
            GetTextBlock()[index + 14].strText.assign("");
    }
    else
        GetTextBlock()[index + 14].strText.assign("");
}

void LobbyUI::UpdateGameStartedTexts()
{
    for (int i = 0; i < 6; ++i)
    {
        if(mIsGameStarted[i])
            GetTextBlock()[i + 20].strText.assign("Playing");
        else
            GetTextBlock()[i + 20].strText.assign("");
    }
}

void LobbyUI::UpdateGameStartedTextsIndex(int index, bool isGameStarted)
{
    if (isGameStarted) 
    {
        GetTextBlock()[index + 20].strText.assign("Playing");
        mIsGameStarted[index] = isGameStarted;
    }
    else 
    {
        GetTextBlock()[index + 20].strText.assign("");
        mIsGameStarted[index] = false;
    }
}

void LobbyUI::UpdateDenyBoxText()
{
    switch (mDenyMessageCode)
    {
    case 1:
        GetTextBlock()[7].strText.assign("Already Started");
        break;
    case 2:
        GetTextBlock()[7].strText.assign("Max Room State");
        break;
    case 3:
        GetTextBlock()[7].strText.assign("Room Is Full");
        break;
    }
}

void LobbyUI::SetRoomInfoTextsIndex(int index, int RoomID, unsigned char PlayerCount, unsigned char MapID, bool GameStarted, bool Opened)
{
    UpdateRoomIsOpenedIndex(index, Opened);
    UpdateRoomIDTextsIndex(index, RoomID, Opened);
    UpdatePlayerCountTextsIndex(index, PlayerCount);
    UpdateMapIDTextsIndex(index, MapID);
    UpdateGameStartedTextsIndex(index, GameStarted);
    RoomEmptyProcess();
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
    for(int i=0;i<26;++i)
    fFontSize.push_back(GetFrameHeight() * 0.03f);
    

    SetFontSize(fFontSize);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Blazed");
    Fonts.push_back(L"Xenogears");
    for(int i=0;i<26-2;++i)
    Fonts.push_back(L"abberancy");
   

    SetFonts(Fonts);

    std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
    TextAlignments.resize(GetTextCnt());
    for(int i=0;i<26;++i)
    TextAlignments[i] = DWRITE_TEXT_ALIGNMENT_CENTER;
    

    SetTextAllignments(TextAlignments);

    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}

void LobbyUI::SetTextRect()
{//MakeRoom, RoomID[6], DenyMessageBox, PlayerCount[6], MapID[6], GameStarted[6]
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.28125f, GetFrameHeight() * 0.055f, GetFrameWidth() * 0.46875f, GetFrameHeight() * 0.11f);
    //RoomID
    GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.245f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.3f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.245f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.3f);
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.425f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.48f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.425f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.48f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.27f, GetFrameHeight() * 0.605f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.66f);
    GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.54f, GetFrameHeight() * 0.605f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.66f);
    //DenyBox
    GetTextBlock()[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.30f, GetFrameHeight() * 0.40f, GetFrameWidth() * 0.70f, GetFrameHeight() * 0.60f);
    //PlayerCount
    GetTextBlock()[8].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.355f, GetFrameWidth() * 0.35f, GetFrameHeight() * 0.39f);
    GetTextBlock()[9].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.355f, GetFrameWidth() * 0.62f, GetFrameHeight() * 0.39f);
    GetTextBlock()[10].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.535f, GetFrameWidth() * 0.35f, GetFrameHeight() * 0.57f);
    GetTextBlock()[11].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.535f, GetFrameWidth() * 0.62f, GetFrameHeight() * 0.57f);
    GetTextBlock()[12].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.715f, GetFrameWidth() * 0.35f, GetFrameHeight() * 0.75f);
    GetTextBlock()[13].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.715f, GetFrameWidth() * 0.62f, GetFrameHeight() * 0.75f);
    //MapID
    GetTextBlock()[14].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.325f, GetFrameWidth() * 0.35f, GetFrameHeight() * 0.36f);
    GetTextBlock()[15].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.325f, GetFrameWidth() * 0.62f, GetFrameHeight() * 0.36f);
    GetTextBlock()[16].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.505f, GetFrameWidth() * 0.35f, GetFrameHeight() * 0.54f);
    GetTextBlock()[17].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.505f, GetFrameWidth() * 0.62f, GetFrameHeight() * 0.54f);
    GetTextBlock()[18].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.685f, GetFrameWidth() * 0.35f, GetFrameHeight() * 0.72f);
    GetTextBlock()[19].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.52f, GetFrameHeight() * 0.685f, GetFrameWidth() * 0.62f, GetFrameHeight() * 0.72f);
    //GameStarted
    GetTextBlock()[20].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.39f, GetFrameHeight() * 0.355f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.39f);
    GetTextBlock()[21].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.66f, GetFrameHeight() * 0.355f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.39f);
    GetTextBlock()[22].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.39f, GetFrameHeight() * 0.535f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.57f);
    GetTextBlock()[23].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.66f, GetFrameHeight() * 0.535f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.57f);
    GetTextBlock()[24].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.39f, GetFrameHeight() * 0.715f, GetFrameWidth() * 0.48f, GetFrameHeight() * 0.75f);
    GetTextBlock()[25].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.66f, GetFrameHeight() * 0.715f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.75f);
}

void LobbyUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    std::vector<D2D1::ColorF> colorList;
    /*Text*/
    // MakeRoom, RoomNum[6], DenyMessage, 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::DarkGray, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));  
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.9f));
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

    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); 
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f)); //22
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f)); 
    SetColors(colorList);

    BuildSolidBrush(GetColors());

    SetTextRect();

    GetTextBlock()[0].strText.assign("Make Room");

    //for (int i = 1; i < 5; ++i)
        //SetIndexRoomNums(i-1, i);

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

