
#include "stdafx.h"
#include "LoginUI.h"
LoginUI::LoginUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue) , TextCnt(7), UICnt(9)
    //Text: GameName, Login, ID, IDText, Password, PassWordText
    //UI: LoginBox
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
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
    
    if (IsLoginFail && MouseCollisionCheck(dx, dy, mvTextBlocks[6]))
    {
        IsLoginFail = false;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }
    
    if (MouseCollisionCheck(dx, dy, mvTextBlocks[1])) //Log-in
    {
        mvColors[1].a = 0.5f;
        if (buttonState & MK_LBUTTON)
        {
            
            if (!IsSignup)
            {
                // 로그인 실패하면?
                IsLoginFail = true;
                UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
                return 1;
            }
            else
            {
                //회원가입 실패하면?
                IsLoginFail = true;
                UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
                return 2;
            }
        }
    }
    else if (MouseCollisionCheck(dx, dy, mvTextBlocks[4]))
    {
        if (!IsSignup)
        {
            if (buttonState & MK_LBUTTON)
                IsSignup = true;
            return 0;
        }
        else
        {
            if (buttonState & MK_LBUTTON)
                IsSignup = false;
            return 0;
        }
               
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
        return 2;
    }
    else 
    { 
        mvColors[1].a = 1.0f; 
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
        return 0;
    }

    
}


void LoginUI::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);
    
    if (IsLoginFail && MouseCollisionCheck(dx, dy, mvTextBlocks[6]))
    {
        mvColors[6].a = 0.5f;
        mvColors[12].a = 0.5f;

        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }
    
    if (MouseCollisionCheck(dx, dy, mvTextBlocks[1])) //Log-in
    {
        mvColors[1].a = 0.5f;
        if (buttonState & MK_LBUTTON)
        {
            // 로그인 실패하면?
            IsLoginFail = true;
            //return 1;
        }
    }
    else mvColors[1].a = 1.0f;

    /*if (MouseCollisionCheck(dx, dy, mvTextBlocks[5]) && buttonState)
        exit(0);*/

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[4]))  // Sign-up
    {
        // UI에서 MouseDown됐을 때 반환 값을 주도록 처리
        // 그 반환 값을 통해서 Netptr처리.
        // 여기서 반환 값 줘야 함
        //새로운 UI처리
       
    }
    UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    //return 0;
}

void LoginUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
    float dx = static_cast<float>(x);
    float dy = static_cast<float>(y);

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[1]))
    {
        mvColors[1].a = 0.5f; // Log-in
       /* if (buttonState & MK_LBUTTON)
        {
            IsLoginFail = true;
        }*/
    }
    else mvColors[1].a = 1.0f;

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[5]))
        mvColors[11].a = 0.3f; // X
    else mvColors[11].a = 0.9f;

    if (MouseCollisionCheck(dx, dy, mvTextBlocks[4]))
        mvColors[4].a = 0.3f; // Sign-up
    else mvColors[4].a = 0.8f;

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

    if (IsLoginFail)
    {
        mvColors[6].a = 0.9f;
        mvColors[12].a = 0.9f;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }
    else
    {
        mvColors[6].a = 0.0f;
        mvColors[12].a = 0.0f;
        UI::BuildSolidBrush(UICnt + 1, TextCnt, mvColors);
    }
    if (IsSignup)
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
    }
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
    UI::CreateFontFormat(fFontSize, Fonts, TextCnt, TextAlignments);
}

void LoginUI::Draw(UINT nFrame)
{// BOX, ID, Password, Login, Sign, Close
    XMFLOAT4 RectLTRB[] =
    {
        {
         mfWidth / 60 * 23,
        mfHeight / 64 * 25,
        mfWidth / 40 * 30,
        mfHeight / 128 * 81
        },
        {
            mfWidth / 40 * 25, 
            mfHeight / 64 * 28, 
            mfWidth / 40 * 29, 
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
    XMFLOAT4 FillLTRB[] =
    {
        {
         mfWidth / 60 * 23,
        mfHeight / 64 * 25,
        mfWidth / 40 * 30,
        mfHeight / 128 * 81
        },
        {
            mfWidth / 40 * 25,
            mfHeight / 64 * 28,
            mfWidth / 40 * 29,
            mfHeight / 128 * 71
        },
        {
            mfWidth / 5 * 2,
            mfHeight / 64 * 28,
            mfWidth / 5 *3,
            mfHeight / 128 * 63
        },
        {
            mfWidth / 5 * 2,
            mfHeight / 128 * 64,
            mfWidth / 5 * 3,
            mfHeight / 128*71
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
    bool IsOutlined[9] = {true,true, true, true, true,false,false, false,false};
    UI::BeginDraw(nFrame);
    UI::RoundedRectDraw(RectLTRB, FillLTRB, TextCnt+1, 0, 0, IsOutlined);
    UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    UI::EndDraw(nFrame);
}

void LoginUI::CreateFontFormat()
{
    fFontSize = mfHeight / 22.0f;
   // std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
    Fonts.push_back(L"Vladimir Script 보통");
    Fonts.push_back(L"바탕체");
    Fonts.push_back(L"굴림체");
    Fonts.push_back(L"굴림체");
    Fonts.push_back(L"굴림체");
    Fonts.push_back(L"굴림체");


    //DWRITE_TEXT_ALIGNMENT TextAlignments[6];
    //TextAlignments.resize(TextCnt);
    TextAlignments[0]=DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_LEADING;
    TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_LEADING;
    TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_CENTER;


    UI::CreateFontFormat(fFontSize, Fonts, TextCnt, TextAlignments);
}

void LoginUI::SetTextRect()
{//Text: GameName, Login, ID, Password, sign, close, LoginFail, SignID, SignPWD
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(mfWidth/128 * 63, mfHeight/8 * 2,mfWidth  / 128 * 83, mfHeight / 8 * 3);
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(mfWidth / 40 * 25, mfHeight / 64 * 28, mfWidth / 40 * 29, mfHeight / 128 * 71);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(mfWidth / 200 * 81, mfHeight / 64 * 28,  mfWidth / 200 * 119,  mfHeight / 128 * 63);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(mfWidth / 200 * 81,  mfHeight / 128 * 64, mfWidth / 200 * 119,  mfHeight / 128 * 71);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth / 5 * 2, mfHeight / 128 * 73, mfWidth / 2 , mfHeight / 128 * 80);
    mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(mfWidth / 160 * 117, mfHeight / 256 * 102,  mfWidth / 160 * 119, mfHeight / 256 * 114);
    mvTextBlocks[6].d2dLayoutRect = D2D1::RectF(mfWidth / 5 * 1, mfHeight / 128 * 43, mfWidth / 5 * 2, mfHeight / 128 * 68);

}

void LoginUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[16] = { D2D1::ColorF(D2D1::ColorF::White, 1.0f), D2D1::ColorF(D2D1::ColorF::White, 1.0f), D2D1::ColorF(D2D1::ColorF::White, 0.5f), D2D1::ColorF(D2D1::ColorF::White, 0.5f), D2D1::ColorF(D2D1::ColorF::White, 0.8f), D2D1::ColorF(D2D1::ColorF::White, 1.0f), D2D1::ColorF(D2D1::ColorF::White, 0.0f)/*Text*/, 
        /*UI*/ D2D1::ColorF(D2D1::ColorF::Blue, 0.5f), D2D1::ColorF(D2D1::ColorF::Yellow, 0.5f), D2D1::ColorF(D2D1::ColorF::Blue, 0.1f), D2D1::ColorF(D2D1::ColorF::Blue, 0.1f), D2D1::ColorF(D2D1::ColorF::Red, 0.9f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f), D2D1::ColorF(D2D1::ColorF::Black, 0.0f) };
    for (auto color : colorList)
        mvColors.push_back(color);
    //D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    
    UI::BuildSolidBrush(UICnt+1 - (!IsLoginFail), TextCnt, mvColors);

    SetTextRect();
    for (auto wc : std::string{ "Dynamic Fomula" })
        mvTextBlocks[0].strText.push_back(wc);
    for (auto wc : std::string{ "Log-in" })
        mvTextBlocks[1].strText.push_back(wc);
    for (auto wc : std::string{ "Sign-up" })
        mvTextBlocks[4].strText.push_back(wc);
    for (auto wc : std::string{ "X" })
        mvTextBlocks[5].strText.push_back(wc);

    for (auto wc : std::string{ "Login Fail" })
        mvTextBlocks[6].strText.push_back(wc);
}

void LoginUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void LoginUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    UI::Initialize(device, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}