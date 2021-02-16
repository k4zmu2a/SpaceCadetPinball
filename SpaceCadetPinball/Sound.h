#pragma once
#include "WaveMix.h"

class Sound
{
public:
	static int Init(HINSTANCE hInstance, int voices, void (* someFuncPtr)(int, MIXWAVE*, int));
	static void Enable(int channelFrom, int channelTo, int enableFlag);
	static void Idle();
	static void Activate();
	static void Deactivate();
	static void Close();
	static void PlaySound(MIXWAVE* wavePtr, int minChannel, int maxChannel, unsigned int dwFlags, int16_t loops);
	static MIXWAVE* LoadWaveFile(LPCSTR lpName);
	static void FreeSound(MIXWAVE* wave);
	static void Flush(int channelFrom, int channelTo);
	static void NullCallback(int a1, MIXWAVE* a2, int a3);
	static LRESULT __stdcall SoundCallBackWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
private:
	static int num_channels;
	static HWND wavemix_window;
	static HANDLE pMem;
	static unsigned int enabled_flag;
	static int channel_time[8];
	static MIXWAVE* channel_wavePtr[8];
	static void (* callback_ptr)(int, MIXWAVE*, int);
	static HMODULE HInstance;
};
