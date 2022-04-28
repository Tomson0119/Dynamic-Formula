#include "stdafx.h"
#include "RoomUI.h"
RoomUI::RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    // Text: 11, RoundRect: 12
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI: NicknameBox[8], StartBox, CarSelectBox[2], MapSelectBox[2]
{
    SetTextCnt(11);
    SetRoundRectCnt(12);
    SetBitmapCnt(9);
    SetUICnt();

    SetVectorSize(nFrame);
    for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
        LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
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
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");
    Fonts.push_back(L"Fonts\\abberancy.ttf");

    FontLoad(Fonts);
}

void RoomUI::SetStartOrReady()
{
    GetTextBlock()[0].strText.clear();
    if (mIsRoomMaster)
        GetTextBlock()[0].strText.assign("START");
    else
        GetTextBlock()[0].strText.assign("READY");
}

void RoomUI::Update(float GTime)
{
    for (UINT i = 0; i < GetTextCnt(); ++i)
        GetTextBlock()[i].strText.clear();
    for (auto& wc : std::string{ "Start" })
        GetTextBlock()[0].strText.push_back(wc);
    for (auto& wc : std::string{ "Car" })
        GetTextBlock()[1].strText.push_back(wc);
    for (auto& wc : std::string{ "Map" })
        GetTextBlock()[2].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[0] })
        GetTextBlock()[3].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[1] })
        GetTextBlock()[4].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[2] })
        GetTextBlock()[5].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[3] })
        GetTextBlock()[6].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[4] })
        GetTextBlock()[7].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[5] })
        GetTextBlock()[8].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[6] })
        GetTextBlock()[9].strText.push_back(wc);
    for (auto& wc : std::string{ mNicknames[7] })
        GetTextBlock()[10].strText.push_back(wc);
    
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
        {
        GetFrameWidth() * 0.015f,
        GetFrameHeight() * 0.015f,
        GetFrameWidth() * 0.985f,
        GetFrameHeight() * 0.985f
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
    bool IsOutlined[12] = { false,false,false,false,false,false,false,false,false,false,false,false };
    float aOpacities[9] = { 1.0f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f };

    BeginDraw(nFrame); 
    DrawBmp(LTRB, 0, 1, aOpacities);
    RoundedRectDraw(RectLTRB, FillLTRB, 0, IsOutlined);
    DrawBmp(LTRB, 1, 8, aOpacities);
    TextDraw(GetTextBlock());
    EndDraw(nFrame);
}

void RoomUI::CreateFontFormat()
{
    std::vector<float> fFontSize;
    fFontSize.push_back(GetFrameHeight() * 0.07f);  //Start
    fFontSize.push_back(GetFrameHeight() * 0.07f); //CarSelect
    fFontSize.push_back(GetFrameHeight() * 0.07f); //MapSelect
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
    fFontSize.push_back(GetFrameHeight() * 0.05f);
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
    TextAlignments[8] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[9] = DWRITE_TEXT_ALIGNMENT_CENTER;
    TextAlignments[10] = DWRITE_TEXT_ALIGNMENT_CENTER;
    SetTextAllignments(TextAlignments);

    UI::CreateFontFormat(GetFontSize(), GetFonts(), GetTextAlignment());
}

void RoomUI::SetTextRect()
{//Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.65f, GetFrameWidth() * 0.975f, GetFrameHeight() * 0.8f);
    GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.65f, GetFrameWidth() * 0.255f, GetFrameHeight() * 0.8f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.825f, GetFrameWidth() * 0.23f, GetFrameHeight() * 0.975f);
    
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.075f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.075f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.075f);
    GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.075f);
    GetTextBlock()[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.385f);
    GetTextBlock()[8].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.385f);
    GetTextBlock()[9].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.385f);
    GetTextBlock()[10].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.385f);
}

void RoomUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    //SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    
    std::vector<D2D1::ColorF> colorList;
    /*Text*/
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    colorList.push_back(D2D1::ColorF::FloralWhite);
    colorList.push_back(D2D1::ColorF::Beige);
    colorList.push_back(D2D1::ColorF::Beige);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    colorList.push_back(D2D1::ColorF::Black);
    /*UI*/
    //UI:  StartBox, CarSelectBox[2], NicknameBox[8], MapSelectBox[2]
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Gold , 0.9f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Orange, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Green, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Navy, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Violet, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Pink, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));
    colorList.push_back(D2D1::ColorF(D2D1::ColorF::Green, 1.0f));
    SetColors(colorList);

    BuildSolidBrush(GetColors());

    SetTextRect(); 
    for (auto& wc : std::string{ "Start" })
        GetTextBlock()[0].strText.push_back(wc);
    for (auto& wc : std::string{ "Car" })
        GetTextBlock()[1].strText.push_back(wc);
    for (auto& wc : std::string{ "Map" })
        GetTextBlock()[2].strText.push_back(wc);

    for (auto& wc : std::string{ "Nickname1" })
    {
        GetTextBlock()[3].strText.push_back(wc);
        mNicknames[0].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname2" })
    {
        GetTextBlock()[4].strText.push_back(wc);
        mNicknames[1].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname3" })
    {
        GetTextBlock()[5].strText.push_back(wc);
        mNicknames[2].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname4" })
    {
        GetTextBlock()[6].strText.push_back(wc);
        mNicknames[3].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname5" })
    {
        GetTextBlock()[7].strText.push_back(wc);
        mNicknames[4].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname6" })
    {
        GetTextBlock()[8].strText.push_back(wc);
        mNicknames[5].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname7" })
    {
        GetTextBlock()[9].strText.push_back(wc);
        mNicknames[6].push_back(wc);
    }
    for (auto& wc : std::string{ "Nickname8" })
    {
        GetTextBlock()[10].strText.push_back(wc);
        mNicknames[7].push_back(wc);
    }
}

void RoomUI::Reset()
{
    UI::Reset();
    for (auto& NickName : mNicknames)
        NickName.clear();
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