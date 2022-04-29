#include "stdafx.h"
#include "InGameUI.h"

InGameUI::InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
	: UI(nFrame, device, pd3dCommandQueue),
	  mRunningTime(0),
	  mMyScore(0),
	  mMyRank(0),
	  mMyLap(0),
	  mCurrentSpeed(0.0f)
	// Text: GameTime, LapCnt, Rank, Velocity
	// Ranking credits 8 * 5(Rank, Nickname, Score, Lap, Missile)
    //UI: DraftGage, Item1, Item2
{
	SetTextCnt(47);
	SetRectCnt(4);
	SetBitmapCnt(6);
	SetGradientCnt(1);
	SetUICnt();
    SetVectorSize(nFrame);
	for (int i = 0; i < static_cast<int>(GetBitmapCnt()); ++i)
		LoadBitmapResourceFromFile(GetBitmapFileNames()[i], i);
}

void InGameUI::SetVectorSize(UINT nFrame)
{
    UI::SetVectorSize(nFrame);
    GetTextBlock().resize(GetTextCnt());
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
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf"); //Time
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf"); //LapNum
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Lap
	Fonts.push_back(L"Fonts\\Xenogears.ttf"); // Rank
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Speed
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // km/h
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Score
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf"); //Rank Credits
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf"); 
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");

	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");

	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");

	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");

	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf");

	
	//LTRB.resize(GetBitmapCnt());

	FontLoad(Fonts);
}


void InGameUI::StartPrint(const std::string& strUIText)
{
    GetTextBlock()[GetTextCnt() - 1].strText = strUIText;
}

void InGameUI::AnimateStartSignAnim()
{

}

void InGameUI::StartAnimation()
{
	if (mIsStartUI[3])
	{
		if (mAnimEndTime - mIngameTime > 0.1f)
		{
			if (mOpacities[3] < 1.0f)
				mOpacities[3] += 0.05f;

			if (mOpacities[4] < 1.0f)
				mOpacities[4] += 0.05f;

			if (mOpacities[5] < 1.0f)
				mOpacities[5] += 0.05f;
		}
		else
		{
			mOpacities[3] = 0.0f;
			mOpacities[4] = 0.0f;
			mOpacities[5] = 0.0f;
			
		}
	}
	else if (mIsStartUI[2])
	{
		if (mAnimEndTime - mIngameTime > 0.1f)
		{
			mLTRB[2].y += 0.5f;
			mLTRB[2].w += 0.5f;

			if (mOpacities[2] < 1.0f)
				mOpacities[2] += 0.05f;
		}
		else
		{
			mLTRB[2].x += 0.1f;
			mLTRB[2].z += 0.1f;

			if (mOpacities[2] > 0.0f)
				mOpacities[2] -= 1.5f;
		}
	}
	else if (mIsStartUI[1])
	{
		if (mAnimEndTime - mIngameTime > 0.1f)
		{
			mLTRB[1].y += 0.5f;
			mLTRB[1].w += 0.5f;

			if (mOpacities[1] < 1.0f)
				mOpacities[1] += 0.05f;
		}
		else
		{
			mLTRB[1].y += 0.1f;
			mLTRB[1].w += 0.1f;

			if (mOpacities[1] > 0.0f)
				mOpacities[1] -= 1.5f;
		}
	}
	else if (mIsStartUI[0])
	{
		if (mAnimEndTime - mIngameTime > 0.1f)
		{
			mLTRB[0].y += 0.5f;
			mLTRB[0].w += 0.5f;

			if (mOpacities[0] < 1.0f)
				mOpacities[0] += 0.5f;
		}
		else
		{
			mLTRB[0].x += 0.1f;
			mLTRB[0].z += 0.1f;

			if (mOpacities[0] > 0.0f)
				mOpacities[0] -= 1.5f;
		}
	}
}

void InGameUI::UpdateIngameTime(float Elapsed)
{
	if(mRunningTime>0.0f)
		mRunningTime -= Elapsed;
	int m{}, s{};
	SetTimeMinSec(m, s);
	for (int i = 0; i < static_cast<int>(GetTextCnt()); ++i)
		GetTextBlock()[i].strText.clear();

	if (m < 10)
		GetTextBlock()[0].strText.push_back('0');
	for (auto& str : std::to_string(m))
		GetTextBlock()[0].strText.push_back(str);

	GetTextBlock()[0].strText.push_back(':');
	if (s < 10)
		GetTextBlock()[0].strText.push_back('0');
	for (int i = 0; i < 1 + !(s < 10); ++i)
		GetTextBlock()[0].strText.push_back(std::to_string(s)[i]);
}

void InGameUI::UpdateMyLap()
{
	for (auto& str : std::to_string(mMyLap))
		GetTextBlock()[1].strText.push_back(str);
	//Lap Font Change
	for (auto& str : std::string{ "Lap" })
		GetTextBlock()[2].strText.push_back(str);
}

void InGameUI::UpdateMyRank()
{
	GetTextBlock()[3].strText.push_back(('0' + mMyRank));
	switch (mMyRank)
	{
	case 1:
		GetTextBlock()[3].strText.push_back('s');
		GetTextBlock()[3].strText.push_back('t');
		break;
	case 2:
		GetTextBlock()[3].strText.push_back('n');
		GetTextBlock()[3].strText.push_back('d');
		break;
	case 3:
		GetTextBlock()[3].strText.push_back('r');
		GetTextBlock()[3].strText.push_back('d');
		break;
	default:
		GetTextBlock()[3].strText.push_back('t');
		GetTextBlock()[3].strText.push_back('h');
		break;
	}
}

void InGameUI::UpdateSpeed()
{
	float CurrentSpeed = GetCurrentSpeed();
	if (CurrentSpeed >= 1000.0f)
	{
		for (int i = 0; i < 6; ++i)
			GetTextBlock()[4].strText.push_back(std::to_string(CurrentSpeed)[i]);
	}
	else if (CurrentSpeed >= 100.0f)
	{
		for (int i = 0; i < 5; ++i)
			GetTextBlock()[4].strText.push_back(std::to_string(CurrentSpeed)[i]);
	}
	else if (CurrentSpeed >= 10.0f)
	{
		for (int i = 0; i < 4; ++i)
			GetTextBlock()[4].strText.push_back(std::to_string(CurrentSpeed)[i]);
	}
	else
	{
		for (int i = 0; i < 3; ++i)
			GetTextBlock()[4].strText.push_back(std::to_string(0.0f)[i]);
	}
	for (auto& str : std::string("km/h"))
		GetTextBlock()[5].strText.push_back(str);
}

void InGameUI::UpdateMyScore()
{
	// 텍스트 설정
	GetTextBlock()[6].strText.assign(std::to_string(mMyScore) + "pts");
}

void InGameUI::SetTimeMinSec(int& m, int& s)
{
	m = static_cast<int>(mRunningTime) / 60;
	s = static_cast<int>(mRunningTime) % 60;
}

void InGameUI::UpdateRankCredits()
{
	size_t ExtraTextCnt = 7;
	for (int i = 0;i < ExtraTextCnt; ++i)
		GetTextBlock()[i].strText.clear();
	// Rank, Nickname, Score, Lap, MissileHit
	//Set
	for (int i = 0; i< 8; ++i)
	{
		mRanks[i] = i + 1;
		mUserNicknames[i] = "Nick" + std::to_string(i + 1);
		mScores[i] = i * 800;
		mLaps[i] = i * 2;
		mMissileHits[i] = i * 3;
	}
	for (int i = 0; i< 8; ++i)
	{
		GetTextBlock()[ExtraTextCnt + static_cast<size_t>(i)].strText.assign(std::to_string(mRanks[i]));
		GetTextBlock()[ExtraTextCnt + (1 * 8) + static_cast<size_t>(i)].strText.assign(mUserNicknames[i]);
		GetTextBlock()[ExtraTextCnt + (2 * 8) + static_cast<size_t>(i)].strText.assign(std::to_string(mScores[i]));
		GetTextBlock()[ExtraTextCnt + (3 * 8) + static_cast<size_t>(i)].strText.assign(std::to_string(mLaps[i]));
		GetTextBlock()[ExtraTextCnt + (4 * 8) + static_cast<size_t>(i)].strText.assign(std::to_string(mMissileHits[i]));
	}
	SetIndexColor(50, D2D1::ColorF(D2D1::ColorF::Black, 0.8f));
	BuildSolidBrush(GetColors());
}

void InGameUI::Update(float Elapsed, Player* mPlayer)
{
	//4초를 세는게 문제. @@ 여기 해야 함
	//StartTime Set
	mIngameTime += Elapsed;
	UINT CountdownTime = START_DELAY_TIME;
	if (mIngameTime >= START_DELAY_TIME - 1.0f)
	{
		if (!mIsStartUI[3]) mAnimEndTime = mIngameTime + 1.0f;
		mIsStartUI[3] = true;
	}
	else if (mIngameTime >= START_DELAY_TIME - 2.0f)
	{
		if (!mIsStartUI[2]) mAnimEndTime = mIngameTime + 1.0f;
		mIsStartUI[2] = true;
	}
	else if (mIngameTime >= START_DELAY_TIME - 3.0f)
	{
		if (!mIsStartUI[1]) mAnimEndTime = mIngameTime + 1.0f;
		mIsStartUI[1] = true;
	}
	else if (mIngameTime >= START_DELAY_TIME - 4.0f)
	{
		if (!mIsStartUI[0]) mAnimEndTime = mIngameTime + 1.0f;
		mIsStartUI[0] = true;
	}

	//Time Set
	if (mIngameTime <= CountdownTime)
	{
		StartAnimation();
		return;
	}
	else if (mIngameTime >= mRunningTime)
	{
		//GameEnd
		UpdateRankCredits();
		return;
	}
	else
	{
		for (auto& Opac : mOpacities)
			Opac = 0.0f;
	}
	//UpdateTime
	UpdateIngameTime(Elapsed);
	//UpdateLap
	UpdateMyLap();
	//UpdateMyRank
	UpdateMyRank();
	//UpdateSpeed
	UpdateSpeed();
	//UpdateScore
	UpdateMyScore(); // @@여기 해야함
	//muItemCnt = mPlayer->GetItemNum();
}

void InGameUI::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'S':
			mMyScore += 100;
			break;
		case 'N':
			SetRunningTime(0.0f);
			break;
		case 'L':
			mMyLap += 1;
			break;
		}
	}
}

void InGameUI::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
	if (buttonState)
	{
		for (auto TextBlock : GetTextBlock())
			if (TextBlock.d2dLayoutRect.left<x &&
				TextBlock.d2dLayoutRect.right>x &&
				TextBlock.d2dLayoutRect.top > y &&
				TextBlock.d2dLayoutRect.bottom < y)
				TextBlock.strText.push_back('0');
	}
}

void InGameUI::Draw(UINT nFrame)
{
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
        }, //Item2 UI
		{
			GetFrameWidth() * 0.1f,
			GetFrameHeight() * 0.1f,
			GetFrameWidth() * 0.9f,
			GetFrameHeight() * 0.9f
		} //RankCredits
    };
    XMFLOAT4 FillLTRB[] = 
    { 
        {
			GetFrameWidth()* (3.0f / 16.0f),
			GetFrameHeight() * (5.0f / 6.0f), 
			GetFrameWidth()* (3.0f / 16.0f) + (GetFrameWidth() * (1.0f / 2.0f) - GetFrameWidth() * (3.0f / 16.0f)) * (mDriftGauge/FIXED_FLOAT_LIMIT),
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
        }, //Item2 UI
		{
			GetFrameWidth() * 0.1f,
			GetFrameHeight() * 0.1f,
			GetFrameWidth() * 0.9f,
			GetFrameHeight() * 0.9f
		} //RankCredits
    };
	
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
	bool IsOutlined[4] = { true, true, true, true };

	BeginDraw(nFrame);
	RectDraw(RectLTRB, FillLTRB, 1, IsOutlined);
	DrawBmp(GetLTRB(), 0, 6, mOpacities);
	TextDraw(GetTextBlock());
	EndDraw(nFrame);
}

void InGameUI::CreateFontFormat()
{
    std::vector<std::wstring> Fonts;
	//Manrope-Regular
    Fonts.push_back(L"FivoSans-Regular"); // Time
    Fonts.push_back(L"FivoSans-Regular"); //LapNum
	Fonts.push_back(L"abberancy"); // Lap
    Fonts.push_back(L"Xenogears"); // Rank
    Fonts.push_back(L"abberancy"); // Speed
	Fonts.push_back(L"abberancy"); // km/h
	Fonts.push_back(L"abberancy"); // Score
	for(int i=0;i<40;++i)
		Fonts.push_back(L"FivoSans-Regular"); //RankCredits

	SetFonts(Fonts);

	std::vector<float> fFontSize;
	fFontSize.push_back(GetFrameHeight() * 0.04f); 
	fFontSize.push_back(GetFrameHeight() * 0.06f);
	fFontSize.push_back(GetFrameHeight() * 0.06f);
	fFontSize.push_back(GetFrameHeight() * 0.07f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	for(int i=0;i<40;++i)
		fFontSize.push_back(GetFrameHeight() * 0.05f); //RankCredits
	SetFontSize(fFontSize);

	std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
	TextAlignments.resize(GetTextCnt());
	TextAlignments[0] = DWRITE_TEXT_ALIGNMENT_LEADING;
	TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_LEADING;
	TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_CENTER;
	TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_CENTER;
	TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_CENTER;
	TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	
	for(size_t i=0;i<40;++i)
		TextAlignments[7+i] = DWRITE_TEXT_ALIGNMENT_LEADING; //RankCredits

    UI::CreateFontFormat(GetFontSize(), GetFonts(), TextAlignments);
}

void InGameUI::SetTextRect()
{//Time, LapNum, Lap, Rank, Speed, km/h, Score
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.03f, GetFrameHeight() * 0.17f, GetFrameWidth() * 0.20f, GetFrameHeight() * 0.21f);
	GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(0.0f, GetFrameHeight() * 0.1f, GetFrameWidth() * 0.5f, GetFrameHeight() * 0.14f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.048f, GetFrameHeight() * 0.11f, GetFrameWidth() * 0.16f, GetFrameHeight() * 0.141f);
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.8f, GetFrameHeight() * 0.10f, GetFrameWidth(), GetFrameHeight() * 0.16f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.73f, GetFrameHeight() * 0.86f, GetFrameWidth() * 0.98f, GetFrameHeight() * 0.90f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.73f, GetFrameHeight() * 0.91f, GetFrameWidth() * 0.98f, GetFrameHeight() * 0.95f);
	GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.75f, GetFrameHeight() * 0.17f, GetFrameWidth() * 0.97f, GetFrameHeight() * 0.23f);

	//RankCredits Rank, Ninkname, Score, Lap, MissileHit
	for (int i = 0; i < 8; ++i)
	{
		GetTextBlock()[7+static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.1f, GetFrameHeight() * 0.1f + (i * (GetFrameHeight() * 0.05f)), GetFrameWidth() * 0.5f, GetFrameHeight() * 0.2f + (i * (GetFrameHeight() * 0.05f)));
		GetTextBlock()[15 + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.25f, GetFrameHeight() * 0.1f + (i* (GetFrameHeight() * 0.05f)), GetFrameWidth() * 0.75f, GetFrameHeight() * 0.2f + (i * (GetFrameHeight() * 0.05f)));
		GetTextBlock()[23 + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.4f, GetFrameHeight() * 0.1f + (i * (GetFrameHeight() * 0.05f)), GetFrameWidth() * 0.9f, GetFrameHeight() * 0.2f + (i * (GetFrameHeight() * 0.05f)));
		GetTextBlock()[31 + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.55f, GetFrameHeight() * 0.1f + (i * (GetFrameHeight() * 0.05f)), GetFrameWidth() * 1.0f, GetFrameHeight() * 0.2f + (i * (GetFrameHeight() * 0.05f)));
		GetTextBlock()[39 + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.7f, GetFrameHeight() * 0.1f + (i * (GetFrameHeight() * 0.05f)), GetFrameWidth() * 1.0f, GetFrameHeight() * 0.2f + (i * (GetFrameHeight() * 0.05f)));
	}

}

void InGameUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
	SetFrame(static_cast<float>(nWidth), static_cast<float>(nHeight));

    UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);

	std::vector<XMFLOAT4> bitmapLTRB2;
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.31f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.51f, GetFrameHeight() * 0.6f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.6f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.49f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.69f, GetFrameHeight() * 0.6f });

	SetLTRB(bitmapLTRB2);

    CreateFontFormat();

	const float	gradient_Alpha = 0.6f;
	std::vector<D2D1::ColorF> colorList;
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	//RankCredits
	for(int i=0;i<40;++i)
		colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));

	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));


    //D2D1::ColorF colorList[8] = { D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f),D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::OrangeRed, 1.0f), D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), D2D1::ColorF(D2D1::ColorF::Red, 1.0f), D2D1::ColorF(D2D1::ColorF::Aqua, 1.0f) };
    D2D1::ColorF gradientColors[4] = { D2D1::ColorF(D2D1::ColorF::ForestGreen, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Yellow, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Orange, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Red, gradient_Alpha) };
	SetColors(colorList);

	BuildBrush(GetColors(), 4, gradientColors);
    
    SetTextRect();

	//임시 시간
	SetRunningTime(180.0f);
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