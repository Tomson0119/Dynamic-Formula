#pragma once
#include "stdafx.h"

constexpr int MAX_CHANNEL = 16; // 동시 재생 가능한 사운드 수
constexpr float MAX_VOLUME = 1.0f;
constexpr float MIN_VOLUME = 0.0f;
constexpr float NORMAL_VOLUME = 0.5f;
constexpr int DRIVING_SOUND_FRAME = 48000;
constexpr int DRIVING_REVERSE_SOUND_FRAME = 32000;
constexpr int BRAKE_SKID_SOUND_FRAME = 44100;
constexpr int BIKE_BRAKE_SOUND_FRAME = 44100;
constexpr int DRIFT_SOUND_FRAME = 48000;


constexpr int MAX_SPEED = 250;

constexpr float DRIVING_SOUND_RUNNING_TIME = 3.964f;
constexpr float BRAKE_SKID_SOUND_RUNNING_TIME = 2.705f;
constexpr float BIKE_BRAKE_SOUND_RUNNING_TIME = 3.310f;

enum class SOUND_TRACK {
	BGM1 = 0,
	BGM2,
	DRIVING_ORIGIN,
	DRIFT_ORIGIN,
	DRIVING_REVERSE,
	BRAKE_SKID,
	BIKE_BRAKE,
	BOOSTER,
	BOOSTERBACK,
	MISSILE,
	MISSILE_EXPLOSION
};

class Sound
{
private:
	FMOD_SYSTEM* mSoundSystem;
	std::vector<FMOD_SOUND*> mSoundFile;
	std::vector<FMOD_CHANNEL*> mChannel;

	bool mIsDriftStart = false;
	bool mIsDrift = false;
	bool mIsDriving = false;

	bool mIsDecelerating = false;

public:
	Sound();
	Sound(std::vector<std::string>& SoundFilePath, std::vector<FMOD_MODE>& mode);
	~Sound();
	void InitSound(std::vector<std::string>& SoundFilePath, std::vector<FMOD_MODE>& mode);
	//void SetVolume(float volume);
	void Play(float volume, int channelNum);
	bool PlayCheck(int channelNum);
	void Stop(int channelNum);
	void Update();

	void SetIsDriftStart();
	void SetIsDrift() { if (mIsDrift) mIsDrift = false; else mIsDrift = true; }
	void SetIsDriving() { if (mIsDriving) mIsDriving = false; else mIsDriving = true; }
	void SetIsDecelerating() { if (mIsDecelerating) mIsDecelerating = false; else mIsDecelerating = true; }

	bool GetIsDriftStart() const { return mIsDriftStart; }
	bool GetIsDrift() const { return mIsDrift; }
	bool GetIsDriving() const { return mIsDriving; }
	bool GetIsDecelerating() const { return mIsDecelerating; }
	const std::vector<FMOD_CHANNEL*>& GetChannel() { return mChannel; }
	FMOD_SYSTEM* GetSystem() { return mSoundSystem; }
};

