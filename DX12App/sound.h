#pragma once
#include "stdafx.h"

constexpr int MAX_CHANNEL = 16; // 동시 재생 가능한 사운드 수

class Sound
{
private:
	FMOD_SYSTEM* mSoundSystem;
	std::vector<FMOD_SOUND*> mSoundFile;
	std::vector<FMOD_CHANNEL*> mChannel;
public:
	Sound();
	Sound(std::vector<std::string>& SoundFilePath, std::vector<FMOD_MODE>& mode);
	~Sound();
	void InitSound(std::vector<std::string>& SoundFilePath, std::vector<FMOD_MODE>& mode);
	//void SetVolume(float volume);
	void Play(float volume, int channelNum);
	void Stop(int channelNum);
	void Update();
};

