#pragma once
#include "stdafx.h"

constexpr int MAX_CHANNEL = 16; // 동시 재생 가능한 사운드 수

enum class SOUND_TRACK {
	BGM1 = 0,
	BGM2,
	TEST_EFFECT,
	DRIFT1,
	DRIFT2,
	DRIFT3
};

class Sound
{
private:
	FMOD_SYSTEM* mSoundSystem;
	std::vector<FMOD_SOUND*> mSoundFile;
	std::vector<FMOD_CHANNEL*> mChannel;
	bool mIsDriftStart = false;
	bool mIsDrift = false;
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
	void SetIsDriftStart() { mIsDriftStart = !mIsDriftStart; }
	bool GetIsDriftStart() const { return mIsDriftStart; }
	void SetIsDrift() { mIsDrift= !mIsDrift; }
	bool GetIsDrift() const { return mIsDrift; }
};

