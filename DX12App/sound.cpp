#include "sound.h"

Sound::Sound()
{

}

Sound::Sound(std::vector<std::string>& SoundFilePath, std::vector<FMOD_MODE>& mode)
{
	InitSound(SoundFilePath, mode);
}

Sound::~Sound()
{

}

void Sound::InitSound(std::vector<std::string>& SoundFilePath, std::vector<FMOD_MODE>& mode)
{
	FMOD_System_Create(&mSoundSystem, FMOD_VERSION);
	FMOD_System_Init(mSoundSystem, MAX_CHANNEL, FMOD_INIT_NORMAL, NULL);
	mSoundFile.resize(SoundFilePath.size());
	mChannel.resize(SoundFilePath.size());
	FMOD_RESULT res{};
	for (int i = 0; i < SoundFilePath.size(); ++i)
	{
		res = FMOD_System_CreateSound(mSoundSystem, SoundFilePath[i].c_str(), mode[i], 0, &mSoundFile[i]);
		if (res == FMOD_OK)
			continue;
	}
}

void Sound::Play(float volume, int channelNum)
{	
	FMOD_System_PlaySound(mSoundSystem, mSoundFile[channelNum], NULL, 0, &mChannel[channelNum]);
	FMOD_Channel_SetVolume(mChannel[channelNum], volume);
}

bool Sound::PlayCheck(int channelNum)
{
	FMOD_BOOL IsPlaying;
	FMOD_Channel_IsPlaying(mChannel[channelNum], &IsPlaying);
	if (IsPlaying)
		return true;
	return false;
}

void Sound::Stop(int channelNum)
{
	FMOD_Channel_Stop(mChannel[channelNum]);
}

void Sound::Update()
{
	FMOD_System_Update(mSoundSystem);
}

void Sound::SetIsDriftStart()
{
	if (mIsDriftStart) mIsDriftStart = false; 
	else mIsDriftStart = true;
}