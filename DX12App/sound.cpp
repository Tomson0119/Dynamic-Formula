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
	FMOD_System_CreateDSPByType(mSoundSystem, FMOD_DSP_TYPE_PITCHSHIFT, &mDSP);
	FMOD_System_SetDSPBufferSize(mSoundSystem, 256, 16);

	for (int i = 0; i < SoundFilePath.size(); ++i)
	{
		res = FMOD_System_CreateSound(mSoundSystem, SoundFilePath[i].c_str(), mode[i], 0, &mSoundFile[i]);
		/*if (mode[i] & FMOD_3D )
		{
			FMOD_DSP_OBJECTPAN DSP_Object{};
			FMOD_Channel_SetPosition
		}*/
		FMOD_Channel_AddDSP(mChannel[i], i, mDSP);
		if (res == FMOD_OK)
			continue;
	}
}

void Sound::Play(float volume, int channelNum)
{
	FMOD_System_PlaySound(mSoundSystem, mSoundFile[channelNum], NULL, 0, &mChannel[channelNum]);
	FMOD_Channel_SetVolume(mChannel[channelNum], volume);
}

void Sound::Play3D(int channelNum, const FMOD_VECTOR& channelPos, const FMOD_VECTOR& channelVel)
{
	FMOD_System_PlaySound(mSoundSystem, mSoundFile[channelNum], NULL, 0, &mChannel[channelNum]);
	//FMOD_Channel_SetVolume(mChannel[channelNum], volume);
	FMOD_Channel_Set3DAttributes(mChannel[channelNum], &channelPos, &channelVel);
	FMOD_Channel_Set3DMinMaxDistance(mChannel[channelNum], MAP_3D_SOUND_DISTANCE_MIN, MAP_3D_SOUND_DISTANCE_MAX);
	//FMOD_Channel_Set3DOcclusion(mChannel[channelNum], 1.0f, 1.0f);
}

void Sound::Play3DForPlayer(int channelNum, const FMOD_VECTOR& channelPos, const FMOD_VECTOR& channelVel)
{
	FMOD_System_PlaySound(mSoundSystem, mSoundFile[channelNum], NULL, 0, &mChannel[channelNum]);
	//FMOD_Channel_SetVolume(mChannel[channelNum], volume);
	FMOD_Channel_Set3DAttributes(mChannel[channelNum], &channelPos, &channelVel);
	FMOD_Channel_Set3DMinMaxDistance(mChannel[channelNum], PLAYER_3D_SOUND_DISTANCE_MIN, PLAYER_3D_SOUND_DISTANCE_MAX);
	//FMOD_Channel_Set3DOcclusion(mChannel[channelNum], 1.0f, 1.0f);
}

void Sound::Update3DSoundForPlayer(int channelNum, const FMOD_VECTOR& channelPos, const FMOD_VECTOR& channelVel, int velocity)
{
	FMOD_Channel_Set3DAttributes(mChannel[channelNum], &channelPos, &channelVel);
	FMOD_Channel_SetPitch(mChannel[channelNum], 0.3f + (1.0f * (velocity * (0.8f / MAX_SPEED))));
	if (velocity < MIN_DRIVING_SOUND_SPEED)
	{
		FMOD_Channel_SetPaused(mChannel[channelNum], true);
		SetPlayerSoundPausedTrue(channelNum - static_cast<int>(IngameUI_SOUND_TRACK::PLAYER1));
	}
	else if (GetPlayerSoundPaused(channelNum - static_cast<int>(IngameUI_SOUND_TRACK::PLAYER1)))
	{
		FMOD_System_PlaySound(mSoundSystem, mSoundFile[channelNum], NULL, 0, &mChannel[channelNum]);
		SetPlayerSoundPausedFalse(channelNum - static_cast<int>(IngameUI_SOUND_TRACK::PLAYER1));
	}
}

void Sound::Set3DPos(int channelNum, const FMOD_VECTOR& soundPos, const FMOD_VECTOR& soundVel)
{
	FMOD_Channel_Set3DAttributes(mChannel[channelNum], &soundPos, &soundVel);
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