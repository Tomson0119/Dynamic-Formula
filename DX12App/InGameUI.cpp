#include "stdafx.h"
#include "InGameUI.h"

InGameUI::InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue)
	: UI(nFrame, device, pd3dCommandQueue),
	  mRunningTime(0),
	  mMyScore(0),
	  mMyRank(1),
	  mMyLap(0),
	  mCurrentSpeed(0)
	// Text: GameTime, LapCnt, Rank, Velocity
	// Ranking credits 8 * 5(Rank, Nickname, Score, Lap, Missile)
    //UI: DraftGage, Item1, Item2
{
	SetTextCnt(SCOREBOARD_TEXTCOUNT + TEXTCOUNT);
	SetRectCnt(4);
	SetBitmapCnt(7);
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
	BitmapFileNames.push_back(L"Resources\\ScoreBoardBG.jpeg");

	SetBitmapFileNames(BitmapFileNames);

	std::vector<std::wstring> Fonts;
	Fonts.push_back(L"Fonts\\FivoSans-Regular.otf"); //Time
	Fonts.push_back(L"Fonts\\Xenogears.ttf"); //LapNum
	Fonts.push_back(L"Fonts\\Xenogears.ttf"); // Lap
	Fonts.push_back(L"Fonts\\Xenogears.ttf"); // Rank
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Speed
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // km/h
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Score
	Fonts.push_back(L"Fonts\\abberancy.ttf"); // Warning
	for(int i =0;i<SCOREBOARD_TEXTCOUNT;++i)
		Fonts.push_back(L"Fonts\\FivoSans-Regular.otf"); //Rank Credits
	
	//LTRB.resize(GetBitmapCnt());
	mLTRB.resize(GetBitmapCnt());
	FontLoad(Fonts);
}

void InGameUI::SetTimeMinSec(int& m, int& s)
{
	m = static_cast<int>(mRunningTime) / 60;
	s = static_cast<int>(mRunningTime) % 60;
}

void InGameUI::SetScoreboardInfo(
	int idx, int rank, int score, 
	int lapCount, int hitCount, 
	const std::string& name)
{                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
	mScoreboard[idx].rank = rank;
	mScoreboard[idx].score = score;
	mScoreboard[idx].lapCount = lapCount;
	mScoreboard[idx].hitCount = hitCount;
	mScoreboard[idx].nickname = name;
}

void InGameUI::SortScoreboard()
{
	std::sort(mScoreboard.begin(), mScoreboard.end(),
		[](const Scoreboard& a, const Scoreboard& b)
		{
			return (a.rank < b.rank);
		});
}

void InGameUI::SetInvisibleStateTextUI()
{
	for (int i = 0; i < TEXTCOUNT; ++i)
		SetIndexColor(i, D2D1::ColorF(D2D1::ColorF::White, 0.0f));
	for(int i=0;i<4;++i)
		mIsOutlined[i] = false;
	//SetItemCount(0);
	SetGradientCnt(0);
	SetDriftGauge(0.0f);
	SetItemCount(0);
	BuildSolidBrush(GetColors());
}

void InGameUI::SetVisibleStateTextUI()
{
	SetGradientCnt(1);
	for (int i = 0; i < TEXTCOUNT; ++i)
		SetIndexColor(i, D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	for(int i=2;i<4;++i)
	SetIndexColor(GetTextCnt() + i, D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
	mIsOutlined[0] = true;
	for (int i = 2; i < 4; ++i)
		mIsOutlined[i] = true;
	mBitmapAnimOpacities[6] = 0.0f;
	BuildSolidBrush(GetColors());
}

void InGameUI::Update(float Elapsed, Player* mPlayer)
{
	//Time Set
	if (mIsStartAnim)
	{
		Start321Animation(Elapsed); //321Go!
		return;
	}
	else if (mIsScoreBoard)
	{
		SetScoreBoard(); //Show during 5s
		return;
	}
	else if(mRunningTime<=0.0f)
	{
		mRunningTime = 0.0f;
		return;
	}
	else if (mIsGoAnim)
	{
		
		GoAnimation(Elapsed);
		return;
	}

	for (auto& Opac : mBitmapAnimOpacities)
		Opac = 0.0f;

	mItemCnt = mPlayer->GetItemNum();

	for (int i = 0; i < static_cast<int>(GetTextCnt()); ++i)
		GetTextBlock()[i].strText.clear();

	if (mIsWarning)
		TextUpdateWarning(Elapsed);

	//UpdateTime
	TextUpdateIngameTime(Elapsed);
	//UpdateLap
	TextUpdateMyLap();
	//UpdateMyRank
	TextUpdateMyRank();
	//UpdateSpeed
	TextUpdateSpeed();
	//UpdateScore
	TextUpdateMyScore(); 
	//UpdateItemCount
	SetItemCount(mPlayer->GetItemNum());
}

void InGameUI::GoAnimation(float Elapsed)
{
	mIsStartAnim = false;
	mStartAnimTime = 0.0f;
	for (int i = 0; i < 2; ++i)
		mIsStartUI[i] = false; // 321 Invisible
	for(int i=0;i<2;++i)
		mBitmapAnimOpacities[i] = 0.0f; //321 Invisible
	// Bitmap위치 원래 위치로 조정
	SetBitmapPos();
	

	mGoAnimTime += Elapsed;
	mAnimEndTime = 1.0f;
	if (mGoAnimTime >= 1.0f)
	{
		mIsGoAnim = false;
		mGoAnimTime = 0.0f;
		mIsStartUI[3] = false;
		//UI띄우기
		SetVisibleStateTextUI();
		SetBitmapPos();
	}
	if (mIsStartUI[3])
	{
		mBitmapAnimOpacities[2] = 0.0f;
		if (mAnimEndTime - mGoAnimTime > 0.1f)
		{
			if (mBitmapAnimOpacities[3] < 1.0f)
				mBitmapAnimOpacities[3] += 0.05f;

			if (mBitmapAnimOpacities[4] < 1.0f)
				mBitmapAnimOpacities[4] += 0.05f;

			if (mBitmapAnimOpacities[5] < 1.0f)
				mBitmapAnimOpacities[5] += 0.05f;
		}
	}
}

void InGameUI::Start321Animation(float Elapsed)
{
	SetInvisibleStateTextUI();// UI Invisible
	if(mIsStartAnim)
		mStartAnimTime += Elapsed;
	if (mStartAnimTime >=START_DELAY_TIME)
	{
		mIsStartAnim = false;
		mStartAnimTime = 0.0f;
		for (int i = 0; i < 3; ++i)
			mIsStartUI[i] = false;
		mBitmapAnimOpacities[2] = 0.0f;
		// Bitmap위치 원래 위치로 조정
		SetBitmapPos();
		//UI띄우기
		//SetVisibleStateTextUI();
	}
	else if (mStartAnimTime >= START_DELAY_TIME - 1.0f)
	{
		if (!mIsStartUI[2]) mAnimEndTime = mStartAnimTime + 1.0f;
		mIsStartUI[2] = true;
	}
	else if (mStartAnimTime >= START_DELAY_TIME - 2.0f)
	{
		if (!mIsStartUI[1]) mAnimEndTime = mStartAnimTime + 1.0f;
		mIsStartUI[1] = true;
	}
	else if (mStartAnimTime >= START_DELAY_TIME - 3.0f)
	{
		if (!mIsStartUI[0]) mAnimEndTime = mStartAnimTime + 1.0f;
		mIsStartUI[0] = true;
	}

	if (mIsStartUI[2])
	{
		mBitmapAnimOpacities[1] = 0.0f;
		if (mAnimEndTime - mStartAnimTime > 0.1f)
		{
			mLTRB[2].y += 0.5f;
			mLTRB[2].w += 0.5f;

			if (mBitmapAnimOpacities[2] < 1.0f)
				mBitmapAnimOpacities[2] += 0.05f;
		}
		else
		{
			mLTRB[2].x += 0.1f;
			mLTRB[2].z += 0.1f;

			if (mBitmapAnimOpacities[2] > 0.0f)
				mBitmapAnimOpacities[2] -= 1.5f;
		}
	}
	else if (mIsStartUI[1])
	{
		mBitmapAnimOpacities[0] = 0.0f;
		if (mAnimEndTime - mStartAnimTime > 0.1f)
		{
			mLTRB[1].y += 0.5f;
			mLTRB[1].w += 0.5f;

			if (mBitmapAnimOpacities[1] < 1.0f)
				mBitmapAnimOpacities[1] += 0.05f;
		}
		else
		{
			mLTRB[1].y += 0.1f;
			mLTRB[1].w += 0.1f;

			if (mBitmapAnimOpacities[1] > 0.0f)
				mBitmapAnimOpacities[1] -= 1.5f;
		}
	}
	else if (mIsStartUI[0])
	{
		if (mAnimEndTime - mStartAnimTime > 0.1f)
		{
			mLTRB[0].y += 0.5f;
			mLTRB[0].w += 0.5f;

			if (mBitmapAnimOpacities[0] < 1.0f)
				mBitmapAnimOpacities[0] += 0.5f;
		}
		else
		{
			mLTRB[0].x += 0.1f;
			mLTRB[0].z += 0.1f;

			if (mBitmapAnimOpacities[0] > 0.0f)
				mBitmapAnimOpacities[0] -= 1.5f;
		}
	}
}

void InGameUI::SetScoreBoard()
{
	SetInvisibleStateTextUI();//UI Invisible
	SetScoreBoardTexts();// ScoreBoard Text Input
	mBitmapAnimOpacities[6] = 1.0f;
}

void InGameUI::ShowScoreBoard()
{
	mIsScoreBoard = true; 
}

void InGameUI::ShowGoAnim()
{
	mIsGoAnim = true; 
	mIsStartUI[3] = true;
}

void InGameUI::TextUpdateIngameTime(float Elapsed)
{
	if (mRunningTime > 0.0f)
		mRunningTime -= Elapsed;
	int m{}, s{};
	SetTimeMinSec(m, s);
	

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

void InGameUI::TextUpdateMyLap()
{
	for (auto& str : std::to_string(mMyLap))
		GetTextBlock()[1].strText.push_back(str);
	//Lap Font Change
	GetTextBlock()[2].strText.assign("Lap");
}

void InGameUI::TextUpdateMyRank()
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

void InGameUI::TextUpdateSpeed()
{
	GetTextBlock()[4].strText.assign(std::to_string((int)(mCurrentSpeed / FIXED_FLOAT_LIMIT)));
	for (auto& str : std::string("km/h"))
		GetTextBlock()[5].strText.push_back(str);
}

void InGameUI::TextUpdateMyScore()
{
	// 텍스트 설정
	GetTextBlock()[6].strText.assign(std::to_string(mMyScore) + "p");
}

void InGameUI::SetScoreBoardTexts()
{
	GetTextBlock()[TEXTCOUNT].strText.assign("RANK");
	GetTextBlock()[TEXTCOUNT+1].strText.assign("NICK");
	GetTextBlock()[TEXTCOUNT+2].strText.assign("SCORE");
	GetTextBlock()[TEXTCOUNT+3].strText.assign("LAP");
	GetTextBlock()[TEXTCOUNT+4].strText.assign("MISSILE");
	size_t CategoryCount = 5;
	SetPlayerCount(8);
	//임시 set
	for (int i = 0; i < mScoreboard.size(); ++i)
	{
		if (mScoreboard[i].rank == 0)
			continue;
		GetTextBlock()[static_cast<size_t>(TEXTCOUNT)+CategoryCount + static_cast<size_t>(i)].strText.assign(std::to_string(mScoreboard[i].rank));
		GetTextBlock()[static_cast<size_t>(TEXTCOUNT)+CategoryCount + (1 * 8) + static_cast<size_t>(i)].strText.assign(mScoreboard[i].nickname); //21 22 23...
		GetTextBlock()[static_cast<size_t>(TEXTCOUNT)+CategoryCount + (2 * 8) + static_cast<size_t>(i)].strText.assign(std::to_string(mScoreboard[i].score));// 29 30 31...
		GetTextBlock()[static_cast<size_t>(TEXTCOUNT)+CategoryCount + (3 * 8) + static_cast<size_t>(i)].strText.assign(std::to_string(mScoreboard[i].lapCount));
		GetTextBlock()[static_cast<size_t>(TEXTCOUNT)+CategoryCount + (4 * 8) + static_cast<size_t>(i)].strText.assign(std::to_string(mScoreboard[i].hitCount));
	}
	GetTextBlock()[GetTextCnt()-1].strText.assign("SCORE BOARD");

	SetIndexColor(GetTextCnt() + 1, D2D1::ColorF(D2D1::ColorF::Black, 0.999f));
	SetIndexColor(GetTextCnt() + 2, D2D1::ColorF(D2D1::ColorF::Red, 0.0f));
	SetIndexColor(GetTextCnt()+3, D2D1::ColorF(D2D1::ColorF::Red, 0.0f));
	BuildSolidBrush(GetColors());
}

void InGameUI::TextUpdateWarning(float Elapsed)
{
	float WARNING_DURATION = 7.5f;
	mWarningTime += Elapsed;
	
	GetTextBlock()[7].strText.assign("WRONG WAY");
	if (mWarningTime <= WARNING_DURATION * 0.10f && mWarningAlpha < 1.0f)
		mWarningAlpha += 0.05f;
	else if (mWarningTime < WARNING_DURATION * 0.28f)
		mWarningAlpha = 1.0f;
	else if (mWarningTime < WARNING_DURATION * 0.32f && mWarningAlpha > 0.0f)
		mWarningAlpha -= 0.1f;
	else if (mWarningTime <= WARNING_DURATION * 0.33f)
		mWarningAlpha = 0.0f;
	else if (mWarningTime <= WARNING_DURATION * 0.43f && mWarningAlpha < 1.0f)
		mWarningAlpha += 0.05f;
	else if (mWarningTime < WARNING_DURATION * 0.61f)
		mWarningAlpha = 1.0f;
	else if (mWarningTime < WARNING_DURATION * 0.65f && mWarningAlpha > 0.0f)
		mWarningAlpha -= 0.1f;
	else if (mWarningTime < WARNING_DURATION * 0.66f && mWarningAlpha > 0.0f)
		mWarningAlpha = 0.0f;
	else if (mWarningTime <= WARNING_DURATION * 0.76f && mWarningAlpha < 1.0f)
		mWarningAlpha += 0.05f;
	else if (mWarningTime < WARNING_DURATION * 0.95f)
		mWarningAlpha = 1.0f;
	else if (mWarningTime < WARNING_DURATION * 0.99f && mWarningAlpha > 0.0f)
		mWarningAlpha -= 0.1f;
	else if (mWarningTime >= WARNING_DURATION)
	{
		mWarningAlpha = 0.0f;
		mWarningTime = 0.0f;
		mIsWarning = false;
	}
	SetIndexColor(7, D2D1::ColorF(D2D1::ColorF::Red, mWarningAlpha));
	BuildSolidBrush(GetColors());
}
void InGameUI::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		/*case 'I':
			mMyScore += 100;
			break;*/
		//case 'L':
		//	if (!mIsStartAnim)
		//	{
		//		mIsStartAnim = true;
		//	}
		//	else
		//	{
		//		mIsStartAnim = false;
		//		mStartAnimTime = 0.0f;
		//		for (auto &IsStartUI : mIsStartUI)
		//			IsStartUI = false;
		//		// Bitmap위치 원래 위치로 조정
		//		SetBitmapPos();
		//		//UI띄우기
		//		SetVisibleStateTextUI();
		//	}
		//	break;
		//case 'Y':
		//	if (!mIsScoreBoard)
		//	{
		//		SetInvisibleStateTextUI();
		//		mIsScoreBoard = true;
		//	}
		//	else
		//	{
		//		SetVisibleStateTextUI();
		//		mIsScoreBoard = false;
		//		mItemCnt = 0;
		//	}
		//	break;
		//case 'U':
		//	if (!mIsGoAnim)
		//	{
		//		mIsGoAnim = true;
		//		mIsStartUI[3] = true;
		//	}
		//	else
		//	{
		//		mGoAnimTime = 0.0f;
		//		mIsGoAnim = false;
		//	}
		//	break;
		//case 'L':
		//	mMyLap += 1;
		//	break;
		//case 'O': // warning
		//	if (!mIsWarning)
		//		mIsWarning = true;
		//	else
		//		mIsWarning = false;
		//	break;
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
			GetFrameWidth() * 0.1f,
			GetFrameHeight() * 0.1f,
			GetFrameWidth() * 0.9f,
			GetFrameHeight() * 0.9f
		}, //ScoreBoards
		{
			GetFrameWidth() * (17.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f),
		GetFrameWidth() * (18.0f / 32.0f),
		GetFrameHeight() * (8.0f / 9.0f)
		}, //Item1 UI
		{
			GetFrameWidth() * (19.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f),
		GetFrameWidth() * (20.0f / 32.0f),
		GetFrameHeight() * (8.0f / 9.0f)
		} //Item2 UI
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
			GetFrameWidth() * 0.1f,
			GetFrameHeight() * 0.1f,
			GetFrameWidth() * 0.9f,
			GetFrameHeight() * 0.9f
		}, //ScoreBoards
		{
			GetFrameWidth() * (17.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f),
			GetFrameWidth() * (18.0f / 32.0f),
			GetFrameHeight() * (8.0f / 9.0f)
		}, //Item1 UI
		{
			GetFrameWidth() * (19.0f / 32.0f),
			GetFrameHeight() * (5.0f / 6.0f),
			GetFrameWidth() * (20.0f / 32.0f),
			GetFrameHeight() * (8.0f / 9.0f)
		} //Item2 UI
    };
	
	BeginDraw(nFrame);
	DrawBmp(GetLTRB(), 6, 1, mBitmapAnimOpacities);
	RectDraw(RectLTRB, FillLTRB, 1, mItemCnt, GetGradientCnt(), mIsOutlined);
	RectDraw(RectLTRB, FillLTRB, 0, mIsScoreBoard, GetGradientCnt(), mIsOutlined);
	DrawBmp(GetLTRB(), 0, 6, mBitmapAnimOpacities);
	TextDraw(GetTextBlock());
	EndDraw(nFrame);
}

void InGameUI::CreateFontFormat()
{
    std::vector<std::wstring> Fonts;
	//Manrope-Regular
    Fonts.push_back(L"FivoSans-Regular"); // Time
    Fonts.push_back(L"Xenogears"); //LapNum
	Fonts.push_back(L"Xenogears"); // Lap
    Fonts.push_back(L"Xenogears"); // Rank
    Fonts.push_back(L"abberancy"); // Speed
	Fonts.push_back(L"abberancy"); // km/h
	Fonts.push_back(L"abberancy"); // Score
	Fonts.push_back(L"abberancy"); // Warning

	for(int i=0;i< SCOREBOARD_TEXTCOUNT;++i)
		Fonts.push_back(L"FivoSans-Regular"); //ScoreBoards

	SetFonts(Fonts);

	std::vector<float> fFontSize;
	fFontSize.push_back(GetFrameHeight() * 0.04f); 
	fFontSize.push_back(GetFrameHeight() * 0.07f);
	fFontSize.push_back(GetFrameHeight() * 0.04f);
	fFontSize.push_back(GetFrameHeight() * 0.07f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	fFontSize.push_back(GetFrameHeight() * 0.11f);

	int CategoryCount = 5;

	for (int i = 0; i < CategoryCount; ++i)
		fFontSize.push_back(GetFrameHeight() * 0.045f);

	for(int i=CategoryCount;i<SCOREBOARD_TEXTCOUNT-1;++i)
		fFontSize.push_back(GetFrameHeight() * 0.04f); //ScoreBoards
	fFontSize.push_back(GetFrameHeight() * 0.05f);
	SetFontSize(fFontSize);

	std::vector<DWRITE_TEXT_ALIGNMENT> TextAlignments;
	TextAlignments.resize(GetTextCnt());
	TextAlignments[0] = DWRITE_TEXT_ALIGNMENT_LEADING;
	TextAlignments[1] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	TextAlignments[2] = DWRITE_TEXT_ALIGNMENT_LEADING;
	TextAlignments[3] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	TextAlignments[4] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	TextAlignments[5] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	TextAlignments[6] = DWRITE_TEXT_ALIGNMENT_TRAILING;
	TextAlignments[7] = DWRITE_TEXT_ALIGNMENT_CENTER;
	
	for(size_t i=TEXTCOUNT;i<GetTextCnt();++i)
		TextAlignments[i] = DWRITE_TEXT_ALIGNMENT_CENTER; //ScoreBoards

    UI::CreateFontFormat(GetFontSize(), GetFonts(), TextAlignments);
}

void InGameUI::SetTextRect()
{//Time, LapNum, Lap, Rank, Speed, km/h, Score, WRONG WAY
    GetTextBlock()[0].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.03f, GetFrameHeight() * 0.17f, GetFrameWidth() * 0.20f, GetFrameHeight() * 0.21f);
	GetTextBlock()[1].d2dLayoutRect = D2D1::RectF(-(GetFrameWidth() *0.1f) , GetFrameHeight() * 0.09f, GetFrameWidth() * 0.07f, GetFrameHeight() * 0.13f);
    GetTextBlock()[2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.068f, GetFrameHeight() * 0.11f, GetFrameWidth() * 0.18f, GetFrameHeight() * 0.141f);
    GetTextBlock()[3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.8f, GetFrameHeight() * 0.10f, GetFrameWidth() * 0.9f, GetFrameHeight() * 0.16f);
    GetTextBlock()[4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.73f, GetFrameHeight() * 0.86f, GetFrameWidth() * 0.94f, GetFrameHeight() * 0.90f);
    GetTextBlock()[5].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.73f, GetFrameHeight() * 0.91f, GetFrameWidth() * 0.94f, GetFrameHeight() * 0.95f);
	GetTextBlock()[6].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.70f, GetFrameHeight() * 0.17f, GetFrameWidth() * 0.90f, GetFrameHeight() * 0.23f);
	GetTextBlock()[7].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.15f, GetFrameHeight() * 0.30f, GetFrameWidth() * 0.85f, GetFrameHeight() * 0.70f);

	//ScoreBoards Rank, Ninkname, Score, Lap, MissileHit
	float HeightSize = 0.06f;
	int CategoryCount = 5;
	
	GetTextBlock()[TEXTCOUNT].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.1f, GetFrameHeight() * 0.15f, GetFrameWidth() * 0.20f, GetFrameHeight() * 0.25f);
	GetTextBlock()[TEXTCOUNT+1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.2f, GetFrameHeight() * 0.15f, GetFrameWidth() * 0.50f, GetFrameHeight() * 0.25f);
	GetTextBlock()[TEXTCOUNT+2].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.50f, GetFrameHeight() * 0.15f, GetFrameWidth() * 0.65f, GetFrameHeight() * 0.25f);
	GetTextBlock()[TEXTCOUNT+3].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.65f, GetFrameHeight() * 0.15f, GetFrameWidth() * 0.75f, GetFrameHeight() * 0.25f);
	GetTextBlock()[TEXTCOUNT+4].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.75f, GetFrameHeight() * 0.15f, GetFrameWidth() * 0.90f, GetFrameHeight() * 0.25f);


	for (int i = 0; i < 8; ++i)
	{
		GetTextBlock()[static_cast<size_t>(TEXTCOUNT) + CategoryCount + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.1f, GetFrameHeight() * 0.25f + (i * (GetFrameHeight() * HeightSize)), GetFrameWidth() * 0.20f, GetFrameHeight() * 0.35f + (i * (GetFrameHeight() * HeightSize)));
		GetTextBlock()[2* static_cast<size_t>(TEXTCOUNT) + CategoryCount + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.20f, GetFrameHeight() * 0.25f + (i* (GetFrameHeight() * HeightSize)), GetFrameWidth() * 0.50f, GetFrameHeight() * 0.35f + (i * (GetFrameHeight() * HeightSize)));
		GetTextBlock()[3 * static_cast<size_t>(TEXTCOUNT) + CategoryCount + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.50f, GetFrameHeight() * 0.25f + (i * (GetFrameHeight() * HeightSize)), GetFrameWidth() * 0.65f, GetFrameHeight() * 0.35f + (i * (GetFrameHeight() * HeightSize)));
		GetTextBlock()[4 * static_cast<size_t>(TEXTCOUNT) + CategoryCount + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.65f, GetFrameHeight() * 0.25f + (i * (GetFrameHeight() * HeightSize)), GetFrameWidth() * 0.75f, GetFrameHeight() * 0.35f + (i * (GetFrameHeight() * HeightSize)));
		GetTextBlock()[5 * static_cast<size_t>(TEXTCOUNT) + CategoryCount + static_cast<size_t>(i)].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.75f, GetFrameHeight() * 0.25f + (i * (GetFrameHeight() * HeightSize)), GetFrameWidth() * 0.90f, GetFrameHeight() * 0.35f + (i * (GetFrameHeight() * HeightSize)));
	}
	GetTextBlock()[SCOREBOARD_TEXTCOUNT + TEXTCOUNT - 1].d2dLayoutRect = D2D1::RectF(GetFrameWidth() * 0.30f, GetFrameHeight() * 0.05f, GetFrameWidth() * 0.70f, GetFrameHeight() * 0.2f);
}

void InGameUI::SetBitmapPos()
{
	std::vector<XMFLOAT4> bitmapLTRB2;
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.35f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.55f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.31f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.51f, GetFrameHeight() * 0.6f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.4f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.6f, GetFrameHeight() * 0.6f });
	bitmapLTRB2.push_back({ GetFrameWidth() * 0.49f, GetFrameHeight() * 0.4f, GetFrameWidth() * 0.69f, GetFrameHeight() * 0.6f });
	bitmapLTRB2.push_back({0.0f , 0.0f, GetFrameWidth(), GetFrameHeight()});

	SetLTRB(bitmapLTRB2);
}

void InGameUI::BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
	UI::BuildObjects(ppd3dRenderTargets, nWidth, nHeight);

	SetBitmapPos();

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
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
	//ScoreBoards
	for(int i=0;i<SCOREBOARD_TEXTCOUNT;++i)
		colorList.push_back(D2D1::ColorF(D2D1::ColorF::White, 1.0f));

	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
	colorList.push_back(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));

    //D2D1::ColorF colorList[8] = { D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f),D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f), D2D1::ColorF(D2D1::ColorF::Black, 1.0f), D2D1::ColorF(D2D1::ColorF::OrangeRed, 1.0f), D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), D2D1::ColorF(D2D1::ColorF::Red, 1.0f), D2D1::ColorF(D2D1::ColorF::Aqua, 1.0f) };
    D2D1::ColorF gradientColors[4] = { D2D1::ColorF(D2D1::ColorF::ForestGreen, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Yellow, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Orange, gradient_Alpha), D2D1::ColorF(D2D1::ColorF::Red, gradient_Alpha) };
	SetColors(colorList);
	BuildBrush(GetColors(), 4, gradientColors);
    SetTextRect();
	//임시 시간
	SetInvisibleStateTextUI();
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