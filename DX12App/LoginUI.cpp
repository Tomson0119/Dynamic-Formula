
#include "stdafx.h"
#include "LoginUI.h"
LoginUI::LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue) , TextCnt(7), UICnt(9)
    //Text: GameName, Login, ID, IDText, Password, PassWordText
    //UI: LoginBox
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < mvBitmapFileNames.size(); ++i)
        LoadBitmapResourceFromFile(mvBitmapFileNames[i], i);
}

LoginUI::~LoginUI()
{

}

void LoginUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

}

void LoginUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
    SetBitmapsSize(2);
    vfFontSize.resize(TextCnt);

    mvBitmapFileNames.push_back(L"Resources\\SampleImg.jpg");
    mvBitmapFileNames.push_back(L"Resources\\YellowBackGroundFlag.jpeg");
    FontLoad(L"Fonts\\Blazed.ttf");

    //mvd2dLinearGradientBrush.resize(TextCnt);
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
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }*/

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[6])) // LoginFail
    {
        mvColors[12].a = 0.0f;
        mvColors[6].a = 0.0f;
        SetLoginFail(false);
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[1])) //Log-in
    {
        mvColors[1].a = 0.5f;
        //if (buttonState & MK_LBUTTON)
        //{
        //    
        //    if (!IsSignup)
        //    {
        //        // 로그인 실패하면?
        //        IsLoginFail = true;
        //        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
        //        return 1;
        //    }
        //    else
        //    {
        //        //회원가입 실패하면?
        //        IsLoginFail = true;
        //        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
        //        return 2;
        //    }
        //}
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[5])) // Sign-up
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
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
        return 2;
    }
    /*else 
    { 
        mvColors[1].a = 1.0f; 
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
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
    //    mvColors[6].a = 0.5f;
    //    mvColors[12].a = 0.5f;

    //    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    //}
    
    if (MouseCollisionCheck(dx, dy, mvTextBlocks[1])) //Log-in
    {
        if (buttonState & MK_LBUTTON)
        {
            // 로그인 실패하면?
            //IsLoginFail = true;
            //return 1;
            ;
        }
        //UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }
    else mvColors[1].a = 1.0f;

    /*if (MouseCollisionCheck(dx, dy, mvTextBlocks[5]) && buttonState)
        exit(0);*/

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[5]))  // Sign-up
    {
        // UI에서 MouseDown됐을 때 반환 값을 주도록 처리
        // 그 반환 값을 통해서 Netptr처리.
        // 여기서 반환 값 줘야 함
        //새로운 UI처리
        ;
    }
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    //return 0;
}

void LoginUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[1])) // Login
    {
        mvColors[1].a = 0.5f; // Log-in
       /* if (buttonState & MK_LBUTTON)
        {
            IsLoginFail = true;
        }*/
    }
    else mvColors[1].a = 1.0f;

    if (IsLoginFail)
    {
        if (MouseCollisionCheck(dx, dy, mvTextBlocks[6])) // LoginFail
        {
            mvColors[6].a = 0.3f;
        }
        else mvColors[6].a = 1.0f;
    }
    if (MouseCollisionCheck(dx, dy, mvTextBlocks[4])) //  Exit
        mvColors[4].a = 0.3f; 
    else mvColors[4].a = 1.0f;

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[5])) // Sign-up
        mvColors[5].a = 0.3f; 
    else mvColors[5].a = 1.0f;

    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
}

void LoginUI::Update(float GTime, std::vector <std::string>& Texts)
{
    //mvTextBlocks[3].strText.clear();
    //mvTextBlocks[5].strText.clear();

    if (!Texts[0].empty())
    {
        mvTextBlocks[2].strText.clear();
        mvTextBlocks[2].strText.assign(Texts[0].begin(), Texts[0].end());
    }

    if (!Texts[1].empty()) 
    {
        mvTextBlocks[3].strText.clear();
        mvTextBlocks[3].strText.assign(Texts[1].begin(), Texts[1].end());
    }

    /*if (IsLoginFail)
    {
        mvColors[6].a = 0.9f;
        mvColors[12].a = 0.9f;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }*/
   /* else
    {
        mvColors[6].a = 0.0f;
        mvColors[12].a = 0.0f;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
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
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
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
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }*/
}

void LoginUI::Update(std::vector<std::string>& Texts) 
{
    mvTextBlocks[2].strText.assign(Texts[0].begin(), Texts[0].end());
    mvTextBlocks[3].strText.assign(Texts[1].begin(), Texts[1].end());
}

void LoginUI::ChangeTextAlignment(UINT uNum, UINT uState)
{
    if (uNum == 0) {
        if (uState)
        {
            TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
            mvColors[2].a = 0.5f;
        }
        else
        {
            TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_LEADING;
            mvColors[2].a = 1.0f;
        }
    }
    if (uNum == 1)
    {
        if (uState)
        {
            TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
            mvColors[3].a = 0.5f;
        }
        else
        {
            TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_LEADING;
            mvColors[3].a = 1.0f;
        }
    }
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    UI::CreateFontFormat(vfFontSize, Fonts, TextCnt, TextAlignments);
}

void LoginUI::Draw(UINT nFrame)
{// BOX, Login, Sign, ID, Password, Login, Close
    //RoundedRectOutline
    XMFLOAT4 RectLTRB[] =
    {
        {
         mfWidth / 0.30f,
        mfHeight / 64 * 25,
        mfWidth / 0.70f,
        mfHeight / 128 * 81
        },
        {
            mfWidth * 0.31f, 
            mfHeight / 64 * 28, 
            mfWidth * 0.55f, 
            mfHeight / 128 * 71
        },
        {
            mfWidth / 5 * 2, 
            mfHeight / 64 * 28, 
            mfWidth / 5 * 3, 
            mfHeight / 128 * 63
        },
        {
            mfWidth / 5 * 2, 
            mfHeight / 128 * 64,
            mfWidth / 5 * 3, 
           mfHeight / 128 * 71
        },
        {
            mfWidth / 160 * 117, 
            mfHeight / 256 * 103, 
            mfWidth / 160 * 119, 
            mfHeight / 256 * 115
        },
        {
            mfWidth / 5 * 1,
            mfHeight / 128 * 43,
            mfWidth / 5 * 2,
            mfHeight / 128 * 68
        },
        {
            mfWidth / 5 * 1,
            mfHeight / 128 * 43,
            mfWidth / 5 * 2,
            mfHeight / 128 * 68
        },
        {
            mfWidth / 5 * 2,
            mfHeight / 128 * 43,
            mfWidth / 5 * 3,
            mfHeight / 128 * 68
        },
        {
            mfWidth / 5 * 3,
            mfHeight / 128 * 43,
            mfWidth / 5 * 4,
            mfHeight / 128 * 68
        }
    };
    //RoundedRectFill
    XMFLOAT4 FillLTRB[] =
    {
        {//MainBlackBox
         mfWidth *0.25f,
        mfHeight * 0.375f,
        mfWidth *0.75f,
        mfHeight * 0.625f
        },
        {//LoginBox
            mfWidth * 0.61f,
            mfHeight * 0.43f,
            mfWidth * 0.74f,
            mfHeight * 0.55f
        },
        {//IDBox
            mfWidth * 0.27f,
            mfHeight * 0.43f,
            mfWidth * 0.59f,
            mfHeight * 0.485f
        },
        {//PWDBox
            mfWidth *0.27f,
            mfHeight * 0.49f,
            mfWidth *0.59f,
            mfHeight * 0.545f
        },
        {//Exit
            mfWidth *0.675f,
            mfHeight* 0.38f,
            mfWidth * 0.745f,
            mfHeight * 0.425f
        },
        {//Login-Fail
            mfWidth * 0.40f,
            mfHeight * 0.40f,
            mfWidth * 0.60f,
            mfHeight * 0.60f
        },
        {
            mfWidth / 5 * 1,
            mfHeight / 128 * 43,
            mfWidth / 5 * 2,
            mfHeight / 128 * 68
        },
        {
            mfWidth / 5 * 2,
            mfHeight / 128 * 43,
            mfWidth / 5 * 3,
            mfHeight / 128 * 68
        },
        {
            mfWidth * 0.66f,
            mfHeight / 128 * 43,
            mfWidth * 0.69f,
            mfHeight / 128 * 68
        }
    };
    bool IsOutlined[9] = {true,true, true, true, true,false,false, false,false};
    UI::BeginDraw(nFrame);
    
    //bitmap
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
        }
    };
    float aOpacities[2] = { 0.5f, 1.0f };
    UI::DrawBmp(LTRB,0 , 2, aOpacities);
    UI::RoundedRectDraw(RectLTRB, FillLTRB, TextCnt + 1, 0, 0, IsOutlined);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LoginUI::CreateFontFormat()
{
    fFontSize = mfHeight * 0.04f;
    
    vfFontSize[0] = mfHeight * 0.05f;
    vfFontSize[1] = mfHeight * 0.04f;
    vfFontSize[2] = mfHeight * 0.04f;
    vfFontSize[3] = mfHeight * 0.04f;
    vfFontSize[4] = mfHeight * 0.04f;
    vfFontSize[5] = mfHeight * 0.04f;
    vfFontSize[6] = mfHeight * 0.04f;
    
   // std::vector<std::wstring> Fonts;
    //Goudy Stout
    Fonts.push_back(GetFontName()); // Title logo
    Fonts.push_back(L"Modern No. 20"); // Login
    Fonts.push_back(L"Modern No. 20"); //ID
    Fonts.push_back(L"Modern No. 20"); //PWD
    Fonts.push_back(L"Modern No. 20"); //Sign-up
    Fonts.push_back(L"Modern No. 20"); // Exit
    Fonts.push_back(L"Modern No. 20"); // Login Fail


    //DWRITE_TEXT_ALIGNMENT TextAlignments[6];
    //TextAlignments.resize(TextCnt);
    TextAlignments[0]=  DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_LEADING;
    TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_LEADING;
    TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_CENTER;


    UI::CreateFontFormat(vfFontSize, Fonts, TextCnt, TextAlignments);
}

void LoginUI::SetTextRect()
{//Text: GameName, Login, ID, Password, Exit, sign, LoginFail 
 // SignID, SignPWD
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth * 0.25f, mfHeight * 0.25f, mfWidth  * 0.75f, mfHeight * 0.375f);
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth * 0.625f, mfHeight * 0.4375f, mfWidth * 0.725f, mfHeight * 0.55f);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(mfWidth * 0.401f, mfHeight * 0.4375f,  mfWidth * 0.599f,  mfHeight * 0.499f);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(mfWidth * 0.401f,  mfHeight * 0.501f, mfWidth * 0.599f,  mfHeight * 0.5615f);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 0.68f, mfHeight * 0.35f, mfWidth * 0.75f, mfHeight * 0.45f);

    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth * 0.4f, mfHeight * 0.5635f, mfWidth * 0.52f , mfHeight * 0.625f);
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(mfWidth * 0.41f, mfHeight * 0.40f, mfWidth * 0.59f, mfHeight * 0.60f);
}

void LoginUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    //Logo, 
    D2D1::ColorF colorList[16] = {  D2D1::ColorF(D2D1::ColorF::Tomato , 1.0f), D2D1::ColorF(D2D1::ColorF::White, 1.0f), D2D1::ColorF(D2D1::ColorF::White, 0.5f), D2D1::ColorF(D2D1::ColorF::White, 0.5f), D2D1::ColorF(D2D1::ColorF::White, 1.0f), D2D1::ColorF(D2D1::ColorF::White, 1.0f), D2D1::ColorF(D2D1::ColorF::White, 1.0f)/*Text*/,
        // BigBackGround, SmallBackGround Title logo, Login, ID, PWD, Exit, Sign-up, LoginFail
        /*UI*/  D2D1::ColorF(D2D1::ColorF::Black, 0.9f), D2D1::ColorF(D2D1::ColorF::Red, 0.8f), D2D1::ColorF(D2D1::ColorF::Coral, 0.4f), D2D1::ColorF(D2D1::ColorF::Coral, 0.4f), D2D1::ColorF(D2D1::ColorF::Red, 0.9f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f) };
    //UI - CenterBigBox, LoginBox, IDBox, PWDBox, ExitBox, Login Fail, Sigu-up Fail, Sign-up IDBox, Sign-up PWDBox
    for (auto color : colorList)
        mvColors.push_back(color);
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    
    UI::BuildSolidBrush(UICnt+1 /*- (!IsLoginFail)*/, TextCnt, mvColors);

    SetTextRect();
    for (auto wc : std::string{ "Dynamic Fomula" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::string{ LoginOrSignup })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::string{ "EXT" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::string{LoginOrSignupReverse })
        mvTextBlocks[5].strText.push_back(wc);
    

    for (auto wc : LoginOrSignup + std::string{ "-FAIL" })
        mvTextBlocks[6].strText.push_back(wc);
}

void LoginUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
    mvBitmapFileNames.clear();
}

void LoginUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    UI::Initialize(device, pd3dCommandQueue);
    for (int i = 0; i < mvBitmapFileNames.size(); ++i)
        LoadBitmapResourceFromFile(mvBitmapFileNames[i], i);
    PreDraw(ppd3dRenderTargets, width, height);
}