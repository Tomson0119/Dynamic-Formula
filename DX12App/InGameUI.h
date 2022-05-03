#pragma once

#include "UI.h"

constexpr int START_DELAY_TIME = 3;
constexpr int MAXRECT = 3;
constexpr int SCOREBOARD_TEXTCOUNT = 46;
constexpr int TEXTCOUNT = 8;

struct Scoreboard
{
    int rank;
    int score;
    int lapCount;
    int hitCount;
    std::string nickname;

    Scoreboard()
        : rank(0),
          score(0),
          lapCount(0),
          hitCount(0)
    {
    }
};

class InGameUI : public UI
{
public:
    InGameUI(UINT nFrame, ComPtr<ID3D12Device> device, ID3D12CommandQueue* pd3dCommandQueue);
    virtual ~InGameUI() = default;
    virtual void Update(float Elapsed, Player* mPlayer) override ;
    virtual void SetVectorSize(UINT nFrame) override;
    virtual void Draw(UINT nFrame) override;
    virtual void BuildObjects(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height) override;
    virtual void Reset() override;
    virtual void OnResize(ID3D12Resource** ppd3dRenderTargets, ComPtr<ID3D12Device> device,
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height) override;
    virtual void SetTextRect() override;
    virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
    virtual void OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam) override;
    void CreateFontFormat();
    void Start321Animation(float Elapsed);
    void GoAnimation(float Elapsed);
    void SetScoreBoard();
    std::vector<XMFLOAT4>& GetLTRB() { return mLTRB; }
    void SetLTRB(const std::vector<XMFLOAT4>& Rects) { int i = 0;  for (auto& R : Rects) mLTRB[i++] = R; }

    //For Packet
    //void UpdateItemCnt(bool IsPlus) { if (IsPlus&&muItemCnt<2) ++muItemCnt; else if(!IsPlus && muItemCnt>0) --muItemCnt; }
    void TextUpdateMyScore();
    void TextUpdateMyLap();
    void TextUpdateMyRank();
    void TextUpdateSpeed();
    void TextUpdateIngameTime(float Elapsed);
    void SetScoreBoardTexts();
    void TextUpdateWarning(float Elapsed);

    void SetScoreboardInfo(
        int idx, int rank, int score, 
        int lapCount, int hitCount, 
        const std::string& name);

    void SortScoreboard();
    void SetPlayerCount(int cnt) { mScoreboard.resize(cnt, {}); }

    void SetDriftGauge(int gauge) { mDriftGauge = gauge; }
    void SetRunningTime(float time) { mRunningTime = time; }
    void SetMyScore(int score) { mMyScore = score; }
    void SetMyRank(int rank) { mMyRank = rank; }
    void SetLap(int lap) { mMyLap = lap; }
    void SetSpeed(int Speed) { mCurrentSpeed = Speed; }
    void SetItemCount(int item) { mItemCnt = item; }
    void SetInvisibleStateTextUI();
    void SetVisibleStateTextUI();
    void SetBitmapPos();

    void ShowScoreBoard(); 
    void ShowStartAnim() { mIsStartAnim = true; }
    void ShowWarning() { mIsWarning = true; }
    void ShowGoAnim(); 

    std::mutex& GetMutex() { return mScoreboardMutex; }

    void SetTimeMinSec(int& m, int& s);
    //void SetWarningText() { GetTextBlock()[7].strText.assign("WARNING"); }

private:
    float mRunningTime;

    int mItemCnt = 0;

    float mWarningAlpha = 0.0f;

    std::atomic_int mDriftGauge = 0;

    float mAnimEndTime = 0.0f;
    float mBitmapAnimOpacities[7] = { 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    bool mIsStartUI[4] = { false, false, false, false };
    bool mIsOutlined[4] = { false, true, true, true };

    std::vector<float> mfOpacities;
    std::vector<XMFLOAT4> mLTRB;
    float mWarningTime = 0.0f;
    std::vector<Scoreboard> mScoreboard;
    std::mutex mScoreboardMutex;

    std::atomic_int mMyScore;
    std::atomic_int mMyRank;
    std::atomic_int mMyLap;
    std::atomic_int mCurrentSpeed;

    float mStartAnimTime = 0.0f;
    float mGoAnimTime = 0.0f;
    float mScoreBoardTime = 0.0f;

    std::atomic_bool mIsGoAnim = false;
    std::atomic_bool mIsStartAnim = false;
    std::atomic_bool mIsScoreBoard = false;
    std::atomic_bool mIsWarning = false;
};