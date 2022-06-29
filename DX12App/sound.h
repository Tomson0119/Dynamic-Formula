#pragma once
#include "stdafx.h"

constexpr int MAX_CHANNEL = 16; // 동시 재생 가능한 사운드 수
constexpr float MAX_VOLUME = 1.0f;
constexpr float MIN_VOLUME = 0.0f;
constexpr float NORMAL_VOLUME = 0.5f;
constexpr int DRIVING_SOUND_FRAME = 48000;
constexpr int DRIVING_REVERSE_SOUND_FRAME = 32000;
constexpr int MAX_SPEED = 250;
constexpr float DRIVING_SOUND_RUNNING_TIME = 3.964f;

enum class SOUND_TRACK {
	BGM1 = 0,
	BGM2,
	TEST_EFFECT,
	DRIVING_ORIGIN,
	DRIFT_ORIGIN,
	DRIVING_REVERSE
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

	bool GetIsDriftStart() const { return mIsDriftStart; }
	bool GetIsDrift() const { return mIsDrift; }
	bool GetIsDriving() const { return mIsDriving; }
	const std::vector<FMOD_CHANNEL*>& GetChannel() { return mChannel; }
	FMOD_SYSTEM* GetSystem() { return mSoundSystem; }
};

