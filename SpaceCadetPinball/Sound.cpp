#include "pch.h"
#include "Sound.h"


int Sound::num_channels;
bool Sound::enabled_flag = false;
int* Sound::TimeStamps = nullptr;

bool Sound::Init(int channels, bool enableFlag)
{
#if MIX_PATCHLEVEL > 2
	Mix_Init(MIX_INIT_MID);
#else
    Mix_Init(MIX_INIT_FLUIDSYNTH);
#endif
	auto result = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
	SetChannels(channels);
	Enable(enableFlag);
	return !result;
}

void Sound::Enable(bool enableFlag)
{
	enabled_flag = enableFlag;
	if (!enableFlag)
		Mix_HaltChannel(-1);
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
	delete[] TimeStamps;
	TimeStamps = nullptr;
	Mix_CloseAudio();
	Mix_Quit();
}

void Sound::PlaySound(Mix_Chunk* wavePtr, int time)
{
	if (wavePtr && enabled_flag)
	{
		if (Mix_Playing(-1) == num_channels)
		{
			auto oldestChannel = std::min_element(TimeStamps, TimeStamps + num_channels) - TimeStamps;
			Mix_HaltChannel(oldestChannel);
		}

		auto channel = Mix_PlayChannel(-1, wavePtr, 0);
		if (channel != -1)
			TimeStamps[channel] = time;
	}
}

Mix_Chunk* Sound::LoadWaveFile(const std::string& lpName)
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

void Sound::SetChannels(int channels)
{
	if (channels <= 0)
		channels = 8;

	num_channels = channels;
	delete[] TimeStamps;
	TimeStamps = new int[num_channels]();
	Mix_AllocateChannels(num_channels);
}
