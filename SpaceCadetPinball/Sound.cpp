#include "pch.h"
#include "Sound.h"


#include "pb.h"
#include "pinball.h"
#include "WaveMix.h"
#include "winmain.h"

int Sound::num_channels;
HWND Sound::wavemix_window;
HANDLE Sound::pMem;
unsigned int Sound::enabled_flag;
int Sound::channel_time[8];
MIXWAVE* Sound::channel_wavePtr[8];
void (*Sound::callback_ptr)(int, MIXWAVE*, int);
HMODULE Sound::HInstance;


int Sound::Init(HINSTANCE hInstance, int voices, void (* someFuncPtr)(int, MIXWAVE*, int))
{
	WNDCLASSA WndClass;
	char FileName[300];

	int channelCount = voices;
	if (voices > 8)
		channelCount = 8;
	num_channels = channelCount;
	if (wavemix_window || pMem)
		return 0;
	enabled_flag = -1;
	for (int i = 0; i < channelCount; ++i)
	{
		channel_time[i] = 0;
		channel_wavePtr[i] = nullptr;
	}
	callback_ptr = someFuncPtr;
	if (!someFuncPtr)
		callback_ptr = NullCallback;

	pinball::make_path_name(FileName, "wavemix.inf", 300);
	FILE* wavemixIniFile = nullptr;
	fopen_s(&wavemixIniFile, FileName, "r");
	if (wavemixIniFile)
	{
		fclose(wavemixIniFile);
	}
	else
	{
		/*FT does not have the file, defaults work OK*/
		if (!pb::FullTiltMode)
			MessageBoxA(winmain::hwnd_frame, pinball::get_rc_string(42, 0), pinball::WindowName, 0x2000u);
	}

	WndClass.style = 0;
	WndClass.lpfnWndProc = SoundCallBackWndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = nullptr;
	WndClass.hCursor = nullptr;
	WndClass.hbrBackground = nullptr;
	WndClass.lpszMenuName = nullptr;
	WndClass.lpszClassName = "WaveMixSoundGuy";
	RegisterClassA(&WndClass);
	wavemix_window = CreateWindowExA(
		0,
		"WaveMixSoundGuy",
		nullptr,
		0x80000000,
		0x80000000,
		0,
		0x80000000,
		0,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
	if (!wavemix_window)
		return 0;

	HInstance = hInstance;
	HANDLE hMixSession = WaveMix::Init();
	pMem = hMixSession;
	if (!hMixSession)
		return 0;
	WaveMix::OpenChannel(hMixSession, num_channels, 2u);
	return 1;
}

void Sound::Enable(int channelFrom, int channelTo, int enableFlag)
{
	if (pMem)
	{
		if (channelTo >= num_channels)
			channelTo = num_channels - 1;

		if (channelFrom >= 0 && channelTo < num_channels)
		{
			for (int index = channelFrom; index <= channelTo; ++index)
			{
				int channelFlag = 1 << index;
				if (enableFlag)
				{
					enabled_flag |= channelFlag;
				}
				else
				{
					enabled_flag &= ~channelFlag;
					Flush(index, index);
				}
			}
		}
	}
}

void Sound::Idle()
{
	if (pMem)
		WaveMix::Pump();
}

void Sound::Activate()
{
	if (pMem)
		WaveMix::Activate(pMem, true);
}

void Sound::Deactivate()
{
	if (pMem)
		WaveMix::Activate(pMem, false);
}

void Sound::Close()
{
	if (wavemix_window)
	{
		DestroyWindow(wavemix_window);
		wavemix_window = nullptr;
	}
	if (pMem)
	{
		WaveMix::CloseChannel(pMem, 0, 1);
		WaveMix::CloseSession(pMem);
		pMem = nullptr;
	}
}

void Sound::PlaySound(MIXWAVE* wavePtr, int minChannel, int maxChannel, unsigned int dwFlags, int16_t loops)
{
	MIXPLAYPARAMS mixParams{};

	if (!pMem)
		return;

	if (maxChannel >= num_channels)
		maxChannel = num_channels - 1;
	if (!wavePtr || minChannel < 0 || maxChannel >= num_channels)
		return;

	if ((dwFlags & 0x8000) != 0 && num_channels > 0)
	{
		int index2 = 0;
		bool ok = false;
		while (channel_wavePtr[index2] != wavePtr)
		{
			if (++index2 >= num_channels)
			{
				ok = true;
				break;
			}
		}
		if (!ok)
			return;
	}


	int playChannel = minChannel;
	if (minChannel < maxChannel)
	{
		int curChannel = minChannel;
		do
		{
			++curChannel;
			if ((1 << curChannel) & enabled_flag &&
				channel_time[curChannel] < channel_time[playChannel])
			{
				playChannel = curChannel;
			}
		}
		while (curChannel < maxChannel);
	}

	if ((1 << playChannel) & enabled_flag)
	{
		mixParams.hMixSession = pMem;
		mixParams.hWndNotify = wavemix_window;
		mixParams.dwFlags = dwFlags;
		mixParams.wSize = 28;
		mixParams.wLoops = loops;
		mixParams.iChannel = playChannel;
		mixParams.lpMixWave = wavePtr;

		callback_ptr(1, wavePtr, playChannel);
		channel_time[playChannel] = timeGetTime();
		channel_wavePtr[playChannel] = wavePtr;
		WaveMix::Play(&mixParams);
	}
}

MIXWAVE* Sound::LoadWaveFile(LPCSTR lpName)
{
	return pMem ? WaveMix::OpenWave(pMem, lpName, HInstance, 1u) : nullptr;
}

void Sound::FreeSound(MIXWAVE* wave)
{
	if (wave && pMem)
		WaveMix::FreeWave(pMem, wave);
}

void Sound::Flush(int channelFrom, int channelTo)
{
	if (pMem)
	{
		if (channelTo >= num_channels)
			channelTo = num_channels - 1;

		if (channelFrom >= 0 && channelTo < num_channels)
		{
			for (auto index = channelFrom; index <= channelTo; index++)
			{
				WaveMix::FlushChannel(pMem, index, 0);
				channel_time[index] = 0;
				channel_wavePtr[index] = nullptr;
			}
		}
	}
}

void Sound::NullCallback(int a1, MIXWAVE* a2, int a3)
{
}

LRESULT Sound::SoundCallBackWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg != MM_WOM_DONE)
		return DefWindowProcA(hWnd, Msg, wParam, lParam);

	auto wavePtr = reinterpret_cast<MIXWAVE*>(lParam);
	int channel = -1;
	for (auto index = 0; index < num_channels; ++index)
	{
		if (channel_wavePtr[index] == wavePtr &&
			(channel < 0 || channel_time[index] < channel_time[channel]))
		{
			channel = index;
		}
	}

	if (channel >= 0)
	{
		channel_time[channel] = 0;
		channel_wavePtr[channel] = nullptr;
	}

	callback_ptr(2, wavePtr, channel);
	return 0;
}
