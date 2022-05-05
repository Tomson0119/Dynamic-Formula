#include "stdafx.h"
#include "RoomUI.h"
RoomUI::RoomUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
    // Text: 11, RoundRect: 12
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8]
    //UI: NicknameBox[8], StartBox, CarSelectBox[2], MapSelectBox[2]
{
    SetTextCnt(19);
    SetRoundRectCnt(12);
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
    SetTextCnt(19);
    SetRoundRectCnt(12);
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
    FontLoad(Fonts);
}

void RoomUI::SetAllPlayerNickNames(std::array<std::string, 8>& names)
{
    for (int i = 0; i < 8; ++i)
        mNicknames[i] = names[i];
}

void RoomUI::SetPlayerAdmin(std::array<bool, 8> IsAdmins)
{
    for (int i = 0; i < 8; ++i)
        mIsAdmin[i] = IsAdmins[i];
}

void RoomUI::SetStartOrReady()
{
    GetTextBlock()[0].strText.clear();
    /*if (mIsAdmin)
        GetTextBlock()[0].strText.assign("START");
    else
        GetTextBlock()[0].strText.assign("READY");*/
}

void RoomUI::Update(float GTime)
{
    //for (UINT i = 0; i < GetTextCnt(); ++i)
        //GetTextBlock()[i].strText.clear();
    for (size_t i = 3; i < static_cast<size_t>(GetTextCnt())-8; ++i)
        GetTextBlock()[i].strText.assign(mNicknames[i - 3]);

    GetTextBlock()[0].strText.assign("Ready");
    GetTextBlock()[1].strText.assign("Car");
    GetTextBlock()[2].strText.assign("Map");
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
    for(size_t i=0;i<static_cast<size_t>(GetTextCnt())-3;++i)
        fFontSize.push_back(GetFrameHeight() * 0.05f);
    SetFontSize(fFontSize);

    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Blazed");
    Fonts.push_back(L"Xenogears");
    Fonts.push_back(L"abberancy");
    for(size_t i=0;i<static_cast<size_t>(GetTextCnt())-3;++i)
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

void RoomUI::SetPlayerInfo(int index, char* name, uint8_t color, bool empty, bool ready)
{

}

void RoomUI::SetTextRect()
{//Text: StartOrReady, CarSelect, MapSelect, Nickname[8], RedayOrStart[8]
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
    
    GetTextBlock()[11].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.075f);
    GetTextBlock()[12].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.075f);
    GetTextBlock()[13].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.075f);
    GetTextBlock()[14].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.025f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.075f);
    GetTextBlock()[15].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.025f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.235f, GetFrameHeight() * 0.385f);
    GetTextBlock()[16].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.265f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.475f, GetFrameHeight() * 0.385f);
    GetTextBlock()[17].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.505f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.715f, GetFrameHeight() * 0.385f);
    GetTextBlock()[18].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.745f, GetFrameHeight() * 0.33f, GetFrameWidth() * 0.955f, GetFrameHeight() * 0.385f);
}

void RoomUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    //SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();
    
    std::vector<D2D1::ColorF> colorList;
    /*Text*/
    //Text: StartOrReady, CarSelect, MapSelect, Nickname[8], RedayOrStart[8]
    colorList.push_back(D2D1::ColorF::FloralWhite);
    colorList.push_back(D2D1::ColorF::Beige);
    colorList.push_back(D2D1::ColorF::Beige);
    for(size_t i=0;i<static_cast<size_t>(GetTextCnt())-3;++i)
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
    GetTextBlock()[0].strText.assign("Start");
    GetTextBlock()[1].strText.assign("Car");
    GetTextBlock()[2].strText.assign("Map");
    for (size_t i = 0; i < 8; ++i)
        GetTextBlock()[i + 3].strText.assign("Nickname" + std::to_string(i + 1));

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