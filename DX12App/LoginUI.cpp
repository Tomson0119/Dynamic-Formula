
#include "stdafx.h"
#include "LoginUI.h"
LoginUI::LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    //Text: GameName, Login, ID, IDText, Password, PassWordText
    //UI: LoginBox
    //Text : 7, UI ; 9
{
    SetTextCnt(7);
    SetRectCnt(9);
    SetBitmapCnt(1);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

void LoginUI::SetStatePop(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue *pd3dCommandQueue,
    ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height)
{
    Reset();

    UI::Initialize(device, pd3dCommandQueue);
    SetTextCnt(7);
    SetRectCnt(9);
    SetBitmapCnt(1);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);

    BuildObjects(ppd3dRenderTargets, width, height);
}

void LoginUI::SetVectorSize(UINT nFrame)
{
    UI::SetVectorSize(nFrame);

    std::vector<std::wstring> BitmapFileNames;
    BitmapFileNames.push_back(L"Resources\\YellowBackGroundFlag.jpeg");

    SetBitmapFileNames(BitmapFileNames);

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

void LoginUI::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{

}

bool LoginUI::MouseCollisionCheck(float x, float y, const TextBlock& TB)
{
    if (x<TB.d2dLayoutRect.right && x>TB.d2dLayoutRect.left &&
        y<TB.d2dLayoutRect.bottom && y>TB.d2dLayoutRect.top)
        return true;
    return false;
}

int LoginUI::OnProcessMouseClick(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    
    /*if (IsLoginFail && MouseCollisionCheck(dx, dy, mTextBlocks[6]))
    {
        IsLoginFail = false;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, GetColors());
    }*/

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[6])) // LoginFail
    {
        GetColors()[12].a = 0.0f;
        GetColors()[6].a = 0.0f;
        SetFailBox(false);
        BuildSolidBrush(GetColors());
    }

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1])) //Log-in
    {
        GetColors()[1].a = 0.5f;
        BuildSolidBrush(GetColors());
        return 1;
    }

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[5])) // Sign-up
    {
        return 2;
    }

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[4])) // Exit
    {
        return 3;
    }
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[2])) // ID
    {
        return 4;
    }
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[3])) // PWD
    {
        return 5;
    }
    /*else 
    { 
        GetColors()[1].a = 1.0f; 
        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
        return 0;
    }*/
    return 0;    
}

void LoginUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    
    
}

void LoginUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    auto& sound = GetSound();
    auto& colors = GetColors();
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[2])) // ID
    {
        colors[9].a = 0.5f;
    }
    else
    {
        colors[9].a = 1.0f;

    }
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[3])) // PWD
    {
        colors[10].a = 0.5f;

    }
    else
    {
        colors[10].a = 1.0f;

    }
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1])) // Login
    {
        GetColors()[1].a = 0.5f; // Log-in
        if (!GetIsMouseCollisionLogin())
        {
            sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_SOUND_TRACK::MOUSE_COLLISION));
            SetIsMouseCollisionLoginTrue();
        }
    }
    else 
    {
        GetColors()[1].a = 1.0f;
        SetIsMouseCollisionLoginFalse();
    }
    
    if (mIsLoginFail)
    {
        if (MouseCollisionCheck(dx, dy, GetTextBlock()[6])) // LoginFail
        {
            GetColors()[6].a = 0.3f;

           
        }
        else
        {
            GetColors()[6].a = 1.0f;
            
        }
    }
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[4])) //  Exit
    {
        GetColors()[4].a = 0.3f;
        if (!GetIsMouseCollisionExt())
        {
            sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_SOUND_TRACK::MOUSE_COLLISION));
            SetIsMouseCollisionExtTrue();
        }
    }
    else 
    {
        GetColors()[4].a = 1.0f;
        SetIsMouseCollisionExtFalse();
    }
   
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[5])) // Sign-up
    {
        GetColors()[5].a = 0.3f;
        if (!GetIsMouseCollisionSignup())
        {
            sound.Play(NORMAL_VOLUME, static_cast<int>(LOGINUI_SOUND_TRACK::MOUSE_COLLISION));
            SetIsMouseCollisionSignupTrue();
        }
    }
    else 
    {
        GetColors()[5].a = 1.0f;
        SetIsMouseCollisionSignupFalse();
    }

    BuildSolidBrush(GetColors());
}

void LoginUI::Update(float GTime, std::vector <std::string>& Texts)
{
    if (!Texts[0].empty())
    {
        GetTextBlock()[2].strText.clear();
        GetTextBlock()[2].strText.assign(Texts[0].begin(), Texts[0].end());
    }

    if (!Texts[1].empty()) 
    {
        GetTextBlock()[3].strText.clear();
        GetTextBlock()[3].strText.assign(Texts[1].begin(), Texts[1].end());
    }
    if (mIsLoginFail)
    {
        SetIndexColor(12, D2D1::ColorF(D2D1::ColorF::Black, 1.0f));
        SetIndexColor(6, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    }
    else
    {
        SetIndexColor(12, D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
        SetIndexColor(6, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
    }
    GetSound().Update();
}

void LoginUI::ChangeTextAlignment(UINT uNum, UINT uState)
{
    if (uNum == 0) {
        if (uState)
        {
            GetTextAlignment()[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
            GetColors()[2].a = 0.5f;
        }
        else
        {
            GetTextAlignment()[2] = DWRITE_TEXT_ALIGNMENT_LEADING;
            GetColors()[2].a = 1.0f;
        }
    }
    if (uNum == 1)
    {
        if (uState)
        {
            GetTextAlignment()[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
            GetColors()[3].a = 0.5f;
        }
        else
        {
            GetTextAlignment()[3] = DWRITE_TEXT_ALIGNMENT_LEADING;
            GetColors()[3].a = 1.0f;
        }
    }
    BuildSolidBrush(GetColors());
    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}

void LoginUI::Draw(UINT nFrame)
{// BOX, Login, Sign, ID, Password, Login, Close
    //RoundedRectOutline
    XMFLOAT4 RectLTRB[] =
    {
        {//MainBlackBox
         GetFrameWidth() * 0.25f,
        GetFrameHeight() * 0.375f,
        GetFrameWidth() * 0.75f,
        GetFrameHeight() * 0.625f
        },
        {//LoginBox
            GetFrameWidth() * 0.61f,
            GetFrameHeight() * 0.43f,
            GetFrameWidth() * 0.74f,
            GetFrameHeight() * 0.55f
        },
        {//IDBox
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.43f,
            GetFrameWidth() * 0.59f,
            GetFrameHeight() * 0.485f
        },
        {//PWDBox
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.49f,
            GetFrameWidth() * 0.59f,
            GetFrameHeight() * 0.545f
        },
        {//Exit
            GetFrameWidth() * 0.675f,
            GetFrameHeight() * 0.38f,
            GetFrameWidth() * 0.745f,
            GetFrameHeight() * 0.425f
        },
        {//Login-Fail
            GetFrameWidth() * 0.40f,
            GetFrameHeight() * 0.40f,
            GetFrameWidth() * 0.60f,
            GetFrameHeight() * 0.60f
        },
        {
            GetFrameWidth() * 0.2f,
            GetFrameHeight() * 0.33f,
            GetFrameWidth() * 0.4f,
            GetFrameHeight() * 0.52f
        },
        {
            GetFrameWidth() * 0.4f,
            GetFrameHeight() * 0.33f,
            GetFrameWidth() * 0.6f,
            GetFrameHeight() * 0.52f
        },
        {
            GetFrameWidth() * 0.66f,
            GetFrameHeight() * 0.33f,
            GetFrameWidth() * 0.69f,
            GetFrameHeight() * 0.52f
        }
    };
    //RoundedRectFill
    XMFLOAT4 FillLTRB[] =
    {
        {//MainBlackBox
         GetFrameWidth() *0.25f,
        GetFrameHeight() * 0.375f,
        GetFrameWidth() *0.75f,
        GetFrameHeight() * 0.625f
        },
        {//LoginBox
            GetFrameWidth() * 0.61f,
            GetFrameHeight() * 0.43f,
            GetFrameWidth() * 0.74f,
            GetFrameHeight() * 0.55f
        },
        {//IDBox
            GetFrameWidth() * 0.27f,
            GetFrameHeight() * 0.43f,
            GetFrameWidth() * 0.59f,
            GetFrameHeight() * 0.485f
        },
        {//PWDBox
            GetFrameWidth() *0.27f,
            GetFrameHeight() * 0.49f,
            GetFrameWidth() *0.59f,
            GetFrameHeight() * 0.545f
        },
        {//Exit
            GetFrameWidth() *0.675f,
            GetFrameHeight()* 0.38f,
            GetFrameWidth() * 0.745f,
            GetFrameHeight() * 0.425f
        },
        {//Login-Fail
            GetFrameWidth() * 0.40f,
            GetFrameHeight() * 0.40f,
            GetFrameWidth() * 0.60f,
            GetFrameHeight() * 0.60f
        },
        {
            GetFrameWidth() * 0.2f,
            GetFrameHeight() * 0.33f,
            GetFrameWidth() * 0.4f,
            GetFrameHeight() * 0.52f
        },
        {
            GetFrameWidth() * 0.4f,
            GetFrameHeight() * 0.33f,
            GetFrameWidth() * 0.6f,
            GetFrameHeight() * 0.52f
        },
        {
            GetFrameWidth() * 0.66f,
            GetFrameHeight() * 0.33f,
            GetFrameWidth() * 0.69f,
            GetFrameHeight() * 0.52f
        }
    };
    //bitmap
    XMFLOAT4 LTRB[] =
    {
        {
            0.0f,
            0.0f,
            GetFrameWidth(),
            GetFrameHeight()
        }
    };
    float aOpacities[1] = { 1.0f };
    bool IsOutlined[9] = { true,true, true, true, true,false,false, false,false };

    BeginDraw(nFrame);
    DrawBmp(LTRB,0 , 1, aOpacities);
    RectDraw(RectLTRB, FillLTRB, 0, GetRectCnt(), 0, IsOutlined);
    TextDraw(GetTextBlock());
    EndDraw(nFrame);
}

void LoginUI::CreateFontFormat()
{
    std::vector<float> vfFontSizes;
    vfFontSizes.push_back(GetFrameHeight() * 0.05f);
    vfFontSizes.push_back(GetFrameHeight() * 0.04f);
    vfFontSizes.push_back(GetFrameHeight() * 0.04f);
    vfFontSizes.push_back(GetFrameHeight() * 0.04f);
    vfFontSizes.push_back(GetFrameHeight() * 0.04f);
    vfFontSizes.push_back(GetFrameHeight() * 0.04f);
    vfFontSizes.push_back(GetFrameHeight() * 0.04f);
    SetFontSize(vfFontSizes);

    std::vector<std::wstring> Fonts;
    //Goudy Stout, Modern No. 20
    Fonts.push_back(L"Blazed"); // Title logo
    Fonts.push_back(L"Xenogears"); // Login
    Fonts.push_back(L"abberancy"); //ID
    Fonts.push_back(L"abberancy"); //PWD
    Fonts.push_back(L"abberancy"); //Sign-up
    Fonts.push_back(L"abberancy"); // Exit
    Fonts.push_back(L"abberancy"); // Login Fail
    SetFonts(Fonts);

    std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
    TextAlignments.resize(GetTextCnt());
    TextAlignments[0]=  DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_LEADING;
    TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_LEADING;
    TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_CENTER;
    SetTextAllignments(TextAlignments);

    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}

void LoginUI::SetFailMessage(int ResultCode)
{
    switch (ResultCode) {
    case 1://Already Exist
        GetTextBlock()[6].strText.assign("Already Exist");
        break;
    case 2: // Register Success
        GetTextBlock()[6].strText.assign("Register Success");
        break;
    case 3: // Invalid ID Or PWD
        GetTextBlock()[6].strText.assign("Invalid ID or Password");
            break;
    }
}


void LoginUI::SetTextRect()
{//Text: GameName, Login, ID, Password, Exit, sign, LoginFail 
 // SignID, SignPWD
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.25f, GetFrameWidth()  * 0.75f, GetFrameHeight() * 0.375f);
    GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.625f, GetFrameHeight() * 0.4375f, GetFrameWidth() * 0.725f, GetFrameHeight() * 0.55f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.28f, GetFrameHeight() * 0.43f, GetFrameWidth() * 0.58f, GetFrameHeight() * 0.485f);
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.28f, GetFrameHeight() * 0.49f, GetFrameWidth() * 0.58f, GetFrameHeight() * 0.545f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.68f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.45f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.4f, GetFrameHeight() * 0.5635f, GetFrameWidth() * 0.52f, GetFrameHeight() * 0.625f);
    GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.41f, GetFrameHeight() * 0.40f, GetFrameWidth() * 0.59f, GetFrameHeight() * 0.60f);
}

void LoginUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    
    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    std::vector < D2D1::ColorF > colorList;
    /*Text*/
    //Text - SmallBackGround Title logo, Login, ID, PWD, Exit, Sign-up, LoginFail
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Tomato, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.0f));
    /*UI*/
    //UI - CenterBigBox, LoginBox, IDBox, PWDBox, ExitBox, Login Fail, Sigu-up Fail, Sign-up IDBox, Sign-up PWDBox
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 0.8f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Coral, 0.4f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Coral, 0.4f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    SetColors(colorList);
    
    BuildSolidBrush(GetColors());

    SetTextRect();
    GetTextBlock()[0].strText.assign("Dynamic Fomula");
    GetTextBlock()[1].strText.assign("Login");
    GetTextBlock()[4].strText.assign("EXT");
    GetTextBlock()[5].strText.assign("Signup");
    GetTextBlock()[6].strText.assign("Login-FAIL");
}

void LoginUI::Reset()
{
    UI::Reset();
}

void LoginUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    UI::Initialize(device, pd3dCommandQueue);
    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
    BuildObjects(ppd3dRenderTargets, width, height);
}
