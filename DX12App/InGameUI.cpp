#include "stdafx.h"
#include "InGameUI.h"

InGameUI::InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue), TextCnt(5), UICnt(3)
    // Text: GameTime, LapCnt, Rank, StartCount, Velocity
    //UI: DraftGage, Item1, Item2
{
    SetVectorSize(nFrame, TextCnt);
    Initialize(device, pd3dCommandQueue);
}

InGameUI::~InGameUI()
{

}

void InGameUI::SetVectorSize(UINT nFrame, UINT TextCnt)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(TextCnt);
    //mvd2dLinearGradientBrush.resize(TextCnt);
}

void InGameUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

}
void InGameUI::StartPrint(const std::wstring& strUIText)
{
    mvTextBlocks[TextCnt - 1].strText = strUIText;
}
void InGameUI::Update(float GTime, Player* mPlayer)
{
	/*TextUI.clear();
	TextUI.resize(5);*/
	mvTextBlocks[4].strText.clear();
	//StartTime Set
	UINT Countdown = 3;
	float CountdownTime = 5.0f;
	if (GTime > 4.0f)
	{
		for (auto wc : L"Start")
			mvTextBlocks[4].strText.push_back(wc);
	}
	else if (GTime > 3.0f)
	{
		mvTextBlocks[4].strText.push_back('0' + Countdown - 2);
	}
	else if (GTime > 2.0f)
	{
		mvTextBlocks[4].strText.push_back('0' + Countdown - 1);
	}
	else if (GTime > 1.0f)
	{
		mvTextBlocks[4].strText.push_back('0' + Countdown);
	}


	//Time Set
	if (GTime < CountdownTime)
	{
		StartPrint(mvTextBlocks[4].strText);
		return;
	}
	for(int i=0;i<TextCnt;++i)
		mvTextBlocks[i].strText.clear();

	float LapTime = GTime - CountdownTime;
	UINT Min = 0;
	float Sec = 0.0;
	Min = LapTime / 60.0f;
	Sec = LapTime - (Min * 60.0f);

	if (Min < 10)
		mvTextBlocks[0].strText.push_back('0');
	for (auto wc : std::to_wstring(Min))
		mvTextBlocks[0].strText.push_back(wc);

	mvTextBlocks[0].strText.push_back(':');
	if (Sec < 10)
		mvTextBlocks[0].strText.push_back('0');
	for (int i = 0; i < 3 + !(Sec < 10); ++i)
		mvTextBlocks[0].strText.push_back(std::to_wstring(Sec)[i]);

	//Lap Count Set
	if (static_cast<int>(GTime / 60) > 0)
	{
		for (auto wc : std::to_wstring(static_cast<int>(LapTime / 60)))
			mvTextBlocks[1].strText.push_back(wc);
		for (auto wc : std::wstring{ L"Lap" })
			mvTextBlocks[1].strText.push_back(wc);
	}
	//My Rank
	UINT MyRank = 1;
	mvTextBlocks[2].strText.push_back(('0' + MyRank));

	switch (MyRank % 10)
	{
	case 1:
		mvTextBlocks[2].strText.push_back('s');
		mvTextBlocks[2].strText.push_back('t');
		break;
	case 2:
		mvTextBlocks[2].strText.push_back('n');
		mvTextBlocks[2].strText.push_back('d');
		break;
	case 3:
		mvTextBlocks[2].strText.push_back('r');
		mvTextBlocks[2].strText.push_back('d');
		break;
	default:
		mvTextBlocks[2].strText.push_back('t');
		mvTextBlocks[2].strText.push_back('h');
		break;
	}
	//Speed
	float CurrentSpeed = mPlayer->GetCurrentVelocity();
	if (mPlayer->GetCurrentVelocity() >= 1000.0f)
	{
		for (int i = 0; i < 6; ++i)
			mvTextBlocks[3].strText.push_back(std::to_wstring(mPlayer->GetCurrentVelocity())[i]);
	}
	else if (mPlayer->GetCurrentVelocity() >= 100.0f)
	{
		for (int i = 0; i < 5; ++i)
			mvTextBlocks[3].strText.push_back(std::to_wstring(mPlayer->GetCurrentVelocity())[i]);
	}
	else if (mPlayer->GetCurrentVelocity() >= 10.0f)
	{
		for (int i = 0; i < 4; ++i)
			mvTextBlocks[3].strText.push_back(std::to_wstring(mPlayer->GetCurrentVelocity())[i]);
	}
	else
	{
		for (int i = 0; i < 3; ++i)
			mvTextBlocks[3].strText.push_back(std::to_wstring(0.0f)[i]);
	}
	for (auto wc : std::wstring(L"km/h"))
		mvTextBlocks[3].strText.push_back(wc);

   /* for (int i = 0; i < TextCnt; ++i)
        mvTextBlocks[i].strText = TextUI[i];*/
    if (fDraftGage >= 1.0f)
    {
        fDraftGage = 0.0f;
        if (uItemCnt < 2)
            uItemCnt += 1;
    }
    //DraftGage Set
    SetDraftGage();
}
void InGameUI::Update(float GTime, std::vector<std::string> Texts)
{
	for (auto text : Texts[0])
		mvTextBlocks[2].strText.push_back(text);
}
void InGameUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
	if (buttonState)
	{
		for (auto TextBlock : mvTextBlocks)
			if (TextBlock.d2dLayoutRect.left<x &&
				TextBlock.d2dLayoutRect.right>x &&
				TextBlock.d2dLayoutRect.top > y &&
				TextBlock.d2dLayoutRect.bottom < y)
				TextBlock.strText.push_back('0');
	}
}

void InGameUI::SetDraftGage()
{
    fDraftGage += 0.001f;
}

void InGameUI::Draw(UINT nFrame)
{
	UI::BeginDraw(nFrame);
    XMFLOAT4 RectLTRB[] =
    {
        {
            mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f),
        mfWidth * (3.0f / 16.0f) + (mfWidth * (1.0f / 2.0f) - mfWidth * (3.0f / 16.0f)),
        mfHeight * (8.0f / 9.0f)
        }, //DraftGage
        {
            mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f),
        mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }, //Item1 UI
        {
            mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f),
        mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }//Item2 UI
    };
    XMFLOAT4 FillLTRB[] = 
    { 
        {
            mfWidth * (3.0f / 16.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (3.0f / 16.0f) + (mfWidth * (1.0f / 2.0f) - mfWidth * (3.0f / 16.0f)) * fDraftGage, 
        mfHeight * (8.0f / 9.0f)
        }, //DraftGage
        {
            mfWidth * (17.0f / 32.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (18.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }, //Item1 UI
        {
            mfWidth * (19.0f / 32.0f), mfHeight * (5.0f / 6.0f), 
        mfWidth * (20.0f / 32.0f), mfHeight * (8.0f / 9.0f)
        }//Item2 UI
    };
	UI::RectDraw(RectLTRB, FillLTRB, TextCnt, 2-uItemCnt, 1);
	UI::TextDraw(nFrame, TextCnt, mvTextBlocks);
    //UI::Draw(nFrame, TextCnt, 1, mvTextBlocks, RectLTRB, FillLTRB);
	UI::EndDraw(nFrame);
}

void InGameUI::CreateFontFormat()
{
    float fFontSize = mfHeight / 15.0f;
    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"Tahoma");
    Fonts.push_back(L"Vladimir Script ∫∏≈Î");
    Fonts.push_back(L"πŸ≈¡√º");
    Fonts.push_back(L"±º∏≤√º");
    Fonts.push_back(L"±º∏≤√º");

    UI::CreateFontFormat(fFontSize, Fonts, TextCnt);         
}

void InGameUI::SetTextRect()
{
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(0.0f, 23.0f + mfHeight / 6, mfWidth / 6, 23.0f + (mfHeight / 6));
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(0.0f, 23.0f, mfWidth / 6, mfHeight / 6);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 0.0f, mfWidth, mfHeight / 6);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(5 * (mfWidth / 6), 5 * (mfHeight / 6), mfWidth, mfHeight);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(mfWidth * 1 / 8, mfHeight / 2 - mfHeight * (1 / 11), mfWidth * 7 / 8, mfHeight / 2 + mfHeight * (1 / 11));
}

void InGameUI::PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    mfWidth = static_cast<float>(nWidth);
    mfHeight = static_cast<float>(nHeight);
    UI::PreDraw(ppd3dRenderTargets, nWidth, nHeight);
    CreateFontFormat();

    D2D1::ColorF colorList[8] = { D2D1::ColorF::Black, (0xE12C38, 1.0f), (0xE12C38, 1.0f), D2D1::ColorF::Black, D2D1::ColorF::OrangeRed, D2D1::ColorF::Yellow, D2D1::ColorF::Red, D2D1::ColorF::Aqua };
    D2D1::ColorF gradientColors[4] = { D2D1::ColorF::ForestGreen, D2D1::ColorF::Yellow, D2D1::ColorF::Orange, D2D1::ColorF::Red };
    UI::BuildBrush(UICnt, TextCnt, colorList,  4, gradientColors);
    
    SetTextRect();
}

void InGameUI::Reset()
{
    UI::Reset();
    mvTextBlocks.clear();
}

void InGameUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
    //Reset();
    SetVectorSize(nFrame, TextCnt);
    UI::Initialize(device, pd3dCommandQueue);
    PreDraw(ppd3dRenderTargets, width, height);
}