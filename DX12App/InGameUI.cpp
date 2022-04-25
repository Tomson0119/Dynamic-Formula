#include "stdafx.h"
#include "InGameUI.h"

InGameUI::InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue*
    pd3dCommandQueue) : UI(nFrame, device, pd3dCommandQueue)
	//Text: 5, Rect: 3
    // Text: GameTime, LapCnt, Rank, StartCount, Velocity
    //UI: DraftGage, Item1, Item2
{
	SetTextCnt(6);
	SetRectCnt(3);
	SetBitmapCnt(6);
	SetGradientCnt(1);
	SetUICnt();
    SetVectorSize(nFrame);
    Initialize(device, pd3dCommandQueue);
	for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
		LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

InGameUI::~InGameUI()
{

}

void InGameUI::SetVectorSize(UINT nFrame)
{
    UI::SetVectorSize(nFrame);
    mvTextBlocks.resize(GetTextCnt());
	ResizeFontSize(GetTextCnt());
	ResizeFonts(GetTextCnt());
    //mvd2dLinearGradientBrush.resize(TextCnt);

	std::vector<std::wstring> BitmapFileNames;
	BitmapFileNames.push_back(L"Resources\\3.png");
	BitmapFileNames.push_back(L"Resources\\2.png");
	BitmapFileNames.push_back(L"Resources\\1.png");
	BitmapFileNames.push_back(L"Resources\\G.png");
	BitmapFileNames.push_back(L"Resources\\O.png");
	BitmapFileNames.push_back(L"Resources\\!.png");

	SetBitmapFileNames(BitmapFileNames);

	std::vector<std::wstring> Fonts;
	Fonts.push_back(L"Fonts\\abberancy.ttf"); //Time
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Lap
	Fonts.push_back(L"Fonts\\Xenogears.ttf"); // Rank
	Fonts.push_back(L"Fonts\\abberancy.ttf"); //Speed
	Fonts.push_back(L"Fonts\\abberancy.ttf"); //Speed
	Fonts.push_back(L"Fonts\\Blazed.ttf"); // 321 Go!
	
	//LTRB.resize(GetBitmapCnt());

	FontLoad(Fonts);
}

void InGameUI::Initialize(ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
{

}

void InGameUI::StartPrint(const std::string& strUIText)
{
    mvTextBlocks[GetTextCnt() - 1].strText = strUIText;
}

void InGameUI::StartAnimation(float GTime)
{
	if (mbIsStartUI[3])
	{
		if (AnimEndTime - GTime > 0.1f)
		{
			if (fOpacities[3] < 1.0f)
				fOpacities[3] += 0.05f;

			if (fOpacities[4] < 1.0f)
				fOpacities[4] += 0.05f;

			if (fOpacities[5] < 1.0f)
				fOpacities[5] += 0.05f;
		}
		else
		{
			fOpacities[3] = 0.0f;
			fOpacities[4] = 0.0f;
			fOpacities[5] = 0.0f;
			
		}
	}
	else if (mbIsStartUI[2])
	{
		if (AnimEndTime - GTime > 0.1f)
		{
			LTRB[2].y += 0.5f;
			LTRB[2].w += 0.5f;

			if (fOpacities[2] < 1.0f)
				fOpacities[2] += 0.05f;
		}
		else
		{
			LTRB[2].x += 0.1f;
			LTRB[2].z += 0.1f;

			if (fOpacities[2] > 0.0f)
				fOpacities[2] -= 1.5f;
		}
	}
	else if (mbIsStartUI[1])
	{
		if (AnimEndTime - GTime > 0.1f)
		{
			LTRB[1].y += 0.5f;
			LTRB[1].w += 0.5f;

			if (fOpacities[1] < 1.0f)
				fOpacities[1] += 0.05f;
		}
		else
		{
			LTRB[1].y += 0.1f;
			LTRB[1].w += 0.1f;

			if (fOpacities[1] > 0.0f)
				fOpacities[1] -= 1.5f;
		}
	}
	else if (mbIsStartUI[0])
	{
		if (AnimEndTime - GTime > 0.1f)
		{
			LTRB[0].y += 0.5f;
			LTRB[0].w += 0.5f;

			if (fOpacities[0] < 1.0f)
				fOpacities[0] += 0.05f;
		}
		else
		{
			LTRB[0].x += 0.1f;
			LTRB[0].z += 0.1f;

			if (fOpacities[0] > 0.0f)
				fOpacities[0] -= 1.5f;
		}
	}
}

void InGameUI::Update(float GTime, Player* mPlayer)
{
	/*TextUI.clear();
	TextUI.resize(5);*/
	mvTextBlocks[5].strText.clear();
	//StartTime Set
	UINT Countdown = 3;
	float CountdownTime = START_DELAY_TIME;
	if (GTime > START_DELAY_TIME - 1.5f)
	{
		if (!mbIsStartUI[3]) AnimEndTime = GTime + 1.5f;
		mbIsStartUI[3] = true;
		/*for (auto &wc : L"Go!")
			mvTextBlocks[5].strText.push_back(wc);*/
	}
	else if (GTime > START_DELAY_TIME - 2.5f)
	{
		if (!mbIsStartUI[2]) AnimEndTime = GTime + 1.0f;
		mbIsStartUI[2] = true;
		//mvTextBlocks[5].strText.push_back('0' + Countdown - (Countdown - 1));
	}
	else if (GTime > START_DELAY_TIME - 3.5f)
	{
		if (!mbIsStartUI[1]) AnimEndTime = GTime + 1.0f;
		mbIsStartUI[1] = true;
		//mvTextBlocks[5].strText.push_back('0' + Countdown - (Countdown - 2));
	}
	else if (GTime > START_DELAY_TIME - 4.5f)
	{
		//3
		if (!mbIsStartUI[0]) AnimEndTime = GTime + 1.0f;
		mbIsStartUI[0] = true;
		//mvTextBlocks[5].strText.push_back('0' + Countdown - (Countdown-3));
	}


	//Time Set
	if (GTime < CountdownTime)
	{
		StartPrint(mvTextBlocks[5].strText);
		StartAnimation(GTime);
		return;
	}
	if (mbIsStartUI[3])
	{
		for (auto& Op : mfOpacities)
			Op = 0.0f;
		for (auto& IsStartUI : mbIsStartUI)
			IsStartUI = false;
	}

	/*if (mIsBoost || mIsShootingMissile)
	{
		mItemOffStart = GTime;
		mItemOffEnd = GTime + mItemOffTime;
		mIsBoost = false;
		mIsShootingMissile = false;
		mItemOff = true;
	}
	if (mItemOff)
	{
		if (GTime == mItemOffEnd)
		{
			mItemOffStart = 0.0f;
			mItemOffEnd = 0.0f;
			mItemOff = false;
		}
	}*/
	


	for(int i=0;i<static_cast<int>(GetTextCnt());++i)
		mvTextBlocks[i].strText.clear();

	for (auto& wc : L"Time: ")
		mvTextBlocks[0].strText.push_back(wc);
	float LapTime = GTime - CountdownTime;
	int Min = 0;
	float Sec = 0.0;
	Min = (int)(LapTime / 60.0f);
	Sec = LapTime - (Min * 60.0f);

	if (Min < 10)
		mvTextBlocks[0].strText.push_back('0');
	for (auto &wc : std::to_wstring(Min))
		mvTextBlocks[0].strText.push_back(wc);

	mvTextBlocks[0].strText.push_back(':');
	if (Sec < 10)
		mvTextBlocks[0].strText.push_back('0');
	for (int i = 0; i < 3 + !(Sec < 10); ++i)
		mvTextBlocks[0].strText.push_back(std::to_wstring(Sec)[i]);

	//Lap Count Set
	if (static_cast<int>(GTime / 3) > 0)
	{
		for (auto &wc : std::to_wstring(static_cast<int>(LapTime / 3)))
			mvTextBlocks[1].strText.push_back(wc);
		for (auto &wc : std::wstring{ L"Lap" })
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
	for (auto &wc : std::wstring(L"km/h"))
		mvTextBlocks[4].strText.push_back(wc);

   /* for (int i = 0; i < TextCnt; ++i)
        mvTextBlocks[i].strText = TextUI[i];*/
	
		//DraftGage Set
	//fDriftGauge = mPlayer->GetDriftGauge();
	uItemCnt = mPlayer->GetItemNum();
    SetDraftGage();
}

void InGameUI::Update(float GTime, std::vector<std::string> Texts)
{
	for (auto &text : Texts[0])
		mvTextBlocks[2].strText.push_back(text);
}

void InGameUI::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	/*switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Z':
			if (uItemCnt > 0 && !mItemOff)
			{
				uItemCnt -= 1;
				mIsBoost = true;
			}
			break;
		case 'X':
			if (uItemCnt > 0 && !mItemOff)
			{
				uItemCnt -= 1;
				mIsShootingMissile = true;
			}
		}
	}*/
	/*if (!mItemOff)
	{
		if ((GetAsyncKeyState('Z') & 0x8000) == 1 && uItemCnt > 0)
		{
			uItemCnt -= 1;
			mIsBoost = true;
		}
		if ((GetAsyncKeyState('X') & 0x8000) == 1 && uItemCnt > 0)
		{
			uItemCnt -= 1;
			mIsShootingMissile = true;
		}
	}*/
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
    fDriftGauge += 0.001f;
	if (fDriftGauge > 1.0f)
		fDriftGauge = 0.0f;
}

void InGameUI::Draw(UINT nFrame)
{
	UI::BeginDraw(nFrame);
    XMFLOAT4 RectLTRB[] =
    {
        {
            GetFrameWidth()* (3.0f / 16.0f),
			GetFrameHeight() * (5.0f / 6.0f),
		GetFrameWidth()* (3.0f / 16.0f) + (GetFrameWidth() * (1.0f / 2.0f) - GetFrameWidth() * (3.0f / 16.0f)),
        GetFrameHeight() * (8.0f / 9.0f)
        }, //DriftGauge
        {
			GetFrameWidth()* (17.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f),
		GetFrameWidth()* (18.0f / 32.0f),
		GetFrameHeight() * (8.0f / 9.0f)
        }, //Item1 UI
        {
			GetFrameWidth()* (19.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f),
		GetFrameWidth()* (20.0f / 32.0f),
		GetFrameHeight() * (8.0f / 9.0f)
        }//Item2 UI
    };
    XMFLOAT4 FillLTRB[] = 
    { 
        {
			GetFrameWidth()* (3.0f / 16.0f),
			GetFrameHeight() * (5.0f / 6.0f), 
			GetFrameWidth()* (3.0f / 16.0f) + (GetFrameWidth() * (1.0f / 2.0f) - GetFrameWidth() * (3.0f / 16.0f)) * fDriftGauge,
			GetFrameHeight() * (8.0f / 9.0f)
        }, //DriftGauge
        {
			GetFrameWidth()* (17.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f), 
			GetFrameWidth()* (18.0f / 32.0f),
			GetFrameHeight() * (8.0f / 9.0f)
        }, //Item1 UI
        {
			GetFrameWidth()* (19.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f), 
			GetFrameWidth()* (20.0f / 32.0f),
			GetFrameHeight() * (8.0f / 9.0f)
        }//Item2 UI
    };
	
	bool IsOutlined[3] = { true, true, true };
	XMFLOAT4 LTRB[7] =
	{
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		},
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		},
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		},
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		},
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		},
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		},
		{
			GetFrameWidth() * 0.4f,
			GetFrameHeight() * 0.4f,
			GetFrameWidth() * 0.6f,
			GetFrameHeight() * 0.6f
		}
	};
	/*mfOpacities.push_back(0.0f);
	mfOpacities.push_back(0.0f);
	mfOpacities.push_back(0.0f);
	mfOpacities.push_back(0.0f);*/

	UI::RectDraw(RectLTRB, FillLTRB, 1, IsOutlined);
	UI::DrawBmp(GetLTRB(), 0, 6, fOpacities);
	UI::TextDraw(nFrame, mvTextBlocks);
    //UI::Draw(nFrame, TextCnt, 1, mvTextBlocks, RectLTRB, FillLTRB);
	UI::EndDraw(nFrame);
}

void InGameUI::CreateFontFormat()
{
	
    std::vector<std::wstring> Fonts;
    Fonts.push_back(L"abberancy"); // Time
    Fonts.push_back(L"abberancy"); //Lap
    Fonts.push_back(L"Xenogears"); // Rank
    Fonts.push_back(L"abberancy"); // Speed
	Fonts.push_back(L"abberancy"); // km/h
    Fonts.push_back(L"Blazed"); // 321 Go!

	SetFonts(Fonts);

	std::vector<float> fFontSize;
	fFontSize.push_back(GetFrameHeight() * 0.04f); 
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.07f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.13f);

	SetFontSize(fFontSize);

	std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
	TextAlignments.resize(GetTextCnt());
	TextAlignments[0] = DWRITE_TEXT_ALIGNMENT_LEADING;
	TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_LEADING;
	TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_CENTER;
	TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
	TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_CENTER;
	TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_CENTER;
	//TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_CENTER;

    UI::CreateFontFormat(GetFontSize(), GetFonts(), TextAlignments);
}

void InGameUI::SetTextRect()
{//Time, Lap, Rank, Speed, km/h, 321 Go! GetFrameWidth() * 0.25f, GetFrameHeight() * 0.13f)
    mvTextBlocks[0].d2dLayoutRect = D2D1::RectF(0.0f, GetFrameHeight() * 0.15f, GetFrameWidth() * 0.22f, GetFrameHeight() * 0.19f);
    mvTextBlocks[1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.02f, GetFrameHeight() * 0.11f, GetFrameWidth() * 0.15f, GetFrameHeight() * 0.15f);
    mvTextBlocks[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.8f, 0.0f, GetFrameWidth(), GetFrameHeight() * 0.16f);
    mvTextBlocks[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.73f, GetFrameHeight() * 0.86f, GetFrameWidth() * 0.98f, GetFrameHeight() * 0.90f);
    mvTextBlocks[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.73f, GetFrameHeight() * 0.91f, GetFrameWidth() * 0.98f, GetFrameHeight() * 0.95f);
	mvTextBlocks[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.35f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.65f, GetFrameHeight() * 0.6f);

}

void InGameUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
	SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);

	std::vector<XMFLOAT4> LTRB2;
	LTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	LTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	LTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	LTRB2.push_back({ GetFrameWidth() * 0.31f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.51f, GetFrameHeight() * 0.6f });
	LTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.6f });
	LTRB2.push_back({ GetFrameWidth() * 0.49f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.69f, GetFrameHeight() * 0.6f });

	SetLTRB(LTRB2);

    CreateFontFormat();

	const float	gradient_Alpha = 0.6f;
	std::vector<D2D1::ColorF> colorList;
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::OrangeRed, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Aqua, 1.0f));


    //D2D1::ColorF colorList[8] = { D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f),D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::OrangeRed, 1.0f), D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), D2D1::ColorF(D2D1::ColorF::Red, 1.0f), D2D1::ColorF(D2D1::ColorF::Aqua, 1.0f) };
    D2D1::ColorF gradientColors[4] = { D2D1::ColorF(D2D1::ColorF::ForestGreen, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Yellow, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Orange, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Red, gradient_Alpha) };
	SetColors(colorList);
	UI::BuildBrush(GetColors(), 4, gradientColors);
    
    SetTextRect();
}

void InGameUI::Reset()
{
    UI::Reset();
	GetBitmapFileNames().clear();
}

void InGameUI::OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
    ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height)
{
	UI::Initialize(device, pd3dCommandQueue);
	//Reset();
    SetVectorSize(nFrame);
	for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
		LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
    BuildObjects(ppd3dRenderTargets, width, height);
}