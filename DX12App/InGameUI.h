#pragma once
#include "UI.h"
constexpr int START_DELAY_TIME = 27;
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
    void StartAnimation();
    void StartPrint(const std::string& strUIText);
    std::vector<XMFLOAT4>& GetLTRB() { return mLTRB; }
    void SetLTRB(const std::vector<XMFLOAT4>& Rects) { int i = 0;  for (auto& R : Rects) mLTRB.push_back(R); }

    //For Packet
    //void UpdateItemCnt(bool IsPlus) { if (IsPlus&&muItemCnt<2) ++muItemCnt; else if(!IsPlus && muItemCnt>0) --muItemCnt; }
    void UpdateMyScore();
    void UpdateMyLap();
    void UpdateMyRank();
    void UpdateSpeed();
    void UpdateIngameTime(float Elapsed);
    void UpdateRankCredits();

    void SetDriftGauge(int gauge) { mDriftGauge = gauge; }
    void SetRunningTime(float time) { mRunningTime = time; }
    void SetMyScore(int score) { mMyScore = score; }
    void SetMyRank(int rank) { mMyRank = rank; }
    void SetLap(int lap) { mMyLap = lap; }
    void SetSpeed(float Speed) { mCurrentSpeed = Speed; }
    float GetCurrentSpeed() const { return mCurrentSpeed; }
    void SetScore(int score) { mMyScore = score; }

    void SetTimeMinSec(int& m, int& s);

    void AnimateStartSignAnim();
private:
    float mRunningTime;

    int mItemCnt = 0;
    std::atomic_int mDriftGauge = 0;

    float mAnimEndTime = 0.0f;
    float mOpacities[6] = { 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f};

    bool mIsStartUI[4] = { false, false, false, false };

    std::vector<float> mfOpacities;
    std::vector<XMFLOAT4> mLTRB;

    //For Packet
    std::array<std::atomic_int, MAX_ROOM_CAPACITY> mScores;
    std::array<std::atomic_int, MAX_ROOM_CAPACITY> mRanks;
    std::array<std::atomic_int, MAX_ROOM_CAPACITY> mLaps;
    std::array<std::atomic_int, MAX_ROOM_CAPACITY> mMissileHits;
    std::array<std::string, MAX_ROOM_CAPACITY> mUserNicknames;

    std::atomic_int mMyScore;
    std::atomic_int mMyRank;
    std::atomic_int mMyLap;
    float mCurrentSpeed;
    bool mIsReverse = false;
    float mIngameTime;

    //ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;
};