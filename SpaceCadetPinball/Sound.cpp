#include "pch.h"
#include "Sound.h"


int Sound::num_channels;
unsigned int Sound::enabled_flag = -1;

int Sound::Init(int voices)
{
	int channelCount = voices;
	if (voices > 8)
		channelCount = 8;
	num_channels = channelCount;

	int flags = 0;
#ifdef MUSIC_SDL
	flags |= MIX_INIT_MID;
#endif

	auto init = Mix_Init(flags);
	return Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16LSB, 2, 1024);
}

void Sound::Enable(int channelFrom, int channelTo, int enableFlag)
{
	enabled_flag = enableFlag ? -1 : 0;
}

void Sound::Activate()
{
	Mix_Resume(-1);
}

void Sound::Deactivate()
{
	Mix_Pause(-1);
}

void Sound::Close()
{
	Mix_CloseAudio();
	Mix_Quit();
}

void Sound::PlaySound(Mix_Chunk* wavePtr, int minChannel, int maxChannel, unsigned int dwFlags, int16_t loops)
{
	if (wavePtr && enabled_flag)
		Mix_PlayChannel(-1, wavePtr, loops);
}

Mix_Chunk* Sound::LoadWaveFile(std::string lpName)
{
	auto wavFile = fopen(lpName.c_str(), "r");
	if (!wavFile)
		return nullptr;
	fclose(wavFile);

	return Mix_LoadWAV(lpName.c_str());
}

void Sound::FreeSound(Mix_Chunk* wave)
{
	if (wave)
		Mix_FreeChunk(wave);
}
