#pragma once


class Sound
{
public:
	static bool Init(int channels, bool enableFlag, int volume);
	static void Enable(bool enableFlag);
	static void Activate();
	static void Deactivate();
	static void Close();
	static void PlaySound(Mix_Chunk* wavePtr, int time);
	static Mix_Chunk* LoadWaveFile(const std::string& lpName);
	static void FreeSound(Mix_Chunk* wave);
	static void SetChannels(int channels);
	static void SetVolume(int volume);
private:
	static int num_channels;
	static bool enabled_flag;
	static int* TimeStamps;
	static int Volume;
};
