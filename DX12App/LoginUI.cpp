
#include "stdafx.h"
#include "LoginUI.h"
LoginUI::LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    //Text: GameName, Login, ID, IDText, Password, PassWordText
    //UI: LoginBox
    //Text : 7, UI ; 9
{
    SetTextCnt(7);
    SetRoundRectCnt(9);
    SetBitmapCnt(1);
    SetUICnt();

    SetVectorSize(nFrame);
    Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

LoginUI::~LoginUI()
{

}

void LoginUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

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
    
    /*if (IsLoginFail && MouseCollisionCheck(dx, dy, mvTextBlocks[6]))
    {
        IsLoginFail = false;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, GetColors());
    }*/

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[6])) // LoginFail
    {
        GetColors()[12].a = 0.0f;
        GetColors()[6].a = 0.0f;
        SetLoginFail(false);
        UI::BuildSolidBrush(GetColors());
    }

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1])) //Log-in
    {
        GetColors()[1].a = 0.5f;
        //if (buttonState & MK_LBUTTON)
        //{
        //    
        //    if (!IsSignup)
        //    {
        //        // 로그인 실패하면?
        //        IsLoginFail = true;
        //        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
        //        return 1;
        //    }
        //    else
        //    {
        //        //회원가입 실패하면?
        //        IsLoginFail = true;
        //        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
        //        return 2;
        //    }
        //}
        UI::BuildSolidBrush(GetColors());
    }

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[5])) // Sign-up
    {
        //MouseUp일 때 실행되도록 변경해야 함.
        if (!IsSignup)
        {
            SetLoginOrSignup("Signup");
            SetLoginOrSignupReverse("Login");
        }
        else
        {
            SetLoginOrSignup("Login");
            SetLoginOrSignupReverse("Signup");
        }
        SetSignupBool(!IsSignup);
        UI::BuildSolidBrush(GetColors());
        return 2;
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
    
    //if (/*IsLoginFail &&*/ MouseCollisionCheck(dx, dy, mvTextBlocks[6]))
    //{
    //    GetColors()[6].a = 0.5f;
    //    GetColors()[12].a = 0.5f;

    //    UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
    //}
    
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1])) //Log-in
    {
        if (buttonState & MK_LBUTTON)
        {
            // 로그인 실패하면?
            //IsLoginFail = true;
            //return 1;
            ;
        }
        //UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
    }
    else GetColors()[1].a = 1.0f;

    /*if (MouseCollisionCheck(dx, dy, mvTextBlocks[5]) && buttonState)
        exit(0);*/

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[5]))  // Sign-up
    {
        // UI에서 MouseDown됐을 때 반환 값을 주도록 처리
        // 그 반환 값을 통해서 Netptr처리.
        // 여기서 반환 값 줘야 함
        //새로운 UI처리
        ;
    }
    UI::BuildSolidBrush(GetColors());
    //return 0;
}

void LoginUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[1])) // Login
    {
        GetColors()[1].a = 0.5f; // Log-in
       /* if (buttonState & MK_LBUTTON)
        {
            IsLoginFail = true;
        }*/
    }
    else GetColors()[1].a = 1.0f;

    if (IsLoginFail)
    {
        if (MouseCollisionCheck(dx, dy, GetTextBlock()[6])) // LoginFail
        {
            GetColors()[6].a = 0.3f;
        }
        else GetColors()[6].a = 1.0f;
    }
    if (MouseCollisionCheck(dx, dy, GetTextBlock()[4])) //  Exit
        GetColors()[4].a = 0.3f;
    else GetColors()[4].a = 1.0f;

    if (MouseCollisionCheck(dx, dy, GetTextBlock()[5])) // Sign-up
        GetColors()[5].a = 0.3f;
    else GetColors()[5].a = 1.0f;

    UI::BuildSolidBrush(GetColors());
}

void LoginUI::Update(float GTime, std::vector <std::string>& Texts)
{
    //mvTextBlocks[3].strText.clear();
    //mvTextBlocks[5].strText.clear();

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

    /*if (IsLoginFail)
    {
        GetColors()[6].a = 0.9f;
        GetColors()[12].a = 0.9f;
        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
    }*/
   /* else
    {
        GetColors()[6].a = 0.0f;
        GetColors()[12].a = 0.0f;
        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
    }*/
    /*if (IsSignup)
    {
        mvTextBlocks[1].strText.clear();
        mvTextBlocks[4].strText.clear();
        mvTextBlocks[6].strText.clear();

        for (auto wc : std::string{ "Log-in" })
            mvTextBlocks[4].strText.push_back(wc);
        for (auto wc : std::string{ "Sign-Up" })
            mvTextBlocks[1].strText.push_back(wc);
        for (auto wc : std::string{ "Sign-Up Fail" })
            mvTextBlocks[6].strText.push_back(wc);
        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
    }
    else
    {
        mvTextBlocks[1].strText.clear();
        mvTextBlocks[4].strText.clear();
        mvTextBlocks[6].strText.clear();

        for (auto wc : std::string{ "Log-in" })
            mvTextBlocks[1].strText.push_back(wc);
        for (auto wc : std::string{ "Sign-Up" })
            mvTextBlocks[4].strText.push_back(wc);
        for (auto wc : std::string{ "Login Fail" })
            mvTextBlocks[6].strText.push_back(wc);
        UI::BuildSolidBrush(UICnt + 1, GetTextCnt(), GetColors());
    }*/
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
    UI::BuildSolidBrush(GetColors());
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

    UI::BeginDraw(nFrame);
    UI::DrawBmp(LTRB,0 , 1, aOpacities);
    UI::RectDraw(RectLTRB, FillLTRB, 0, IsOutlined);
    UI::TextDraw(GetTextBlock());
    UI::EndDraw(nFrame);
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
    //Text - BigBackGround, SmallBackGround Title logo, Login, ID, PWD, Exit, Sign-up, LoginFail
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Tomato, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    /*UI*/
    //UI - CenterBigBox, LoginBox, IDBox, PWDBox, ExitBox, Login Fail, Sigu-up Fail, Sign-up IDBox, Sign-up PWDBox
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 0.8f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Coral, 0.4f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Coral, 0.4f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    SetColors(colorList);
    
    UI::BuildSolidBrush(GetColors());

    SetTextRect();
    for (auto wc : std::string{ "Dynamic Fomula" })
        GetTextBlock()[0].strText.push_back(wc);
    for (auto wc : std::string{ LoginOrSignup })
        GetTextBlock()[1].strText.push_back(wc);
    for (auto wc : std::string{ "EXT" })
        GetTextBlock()[4].strText.push_back(wc);
    for (auto wc : std::string{LoginOrSignupReverse })
        GetTextBlock()[5].strText.push_back(wc);
    
    for (auto wc : std::string{ "Login-FAIL" })
        GetTextBlock()[6].strText.push_back(wc);
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