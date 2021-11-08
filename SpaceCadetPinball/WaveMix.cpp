#include "pch.h"
#include "WaveMix.h"

#include "pinball.h"

int WaveMix::initialized_flag;
char WaveMix::FileName[276];
CHANNELNODE WaveMix::channel_nodes[MAXQUEUEDWAVES];
CHANNELNODE* WaveMix::free_channel_nodes;
unsigned char WaveMix::volume_table[11][256];
int WaveMix::debug_flag;
void (*WaveMix::cmixit_ptr)(uint8_t* lpDest, uint8_t** rgWaveSrc, volume_struct* volume, int iNumWaves,
                            uint16_t length);
HMODULE WaveMix::HModule;
PCMWAVEFORMAT WaveMix::gpFormat = {{1u, 1u, 11025u, 11025u, 1u}, 8u};
char WaveMix::string_buffer[256] = "WaveMix V 2.3 by Angel M. Diaz, Jr. (c) Microsoft 1993-1995";
GLOBALS *WaveMix::Globals, *WaveMix::GlobalsActive;
int WaveMix::ShowDebugDialogs;
PLAYQUEUE WaveMix::play_queue;
CHANNELNODE* WaveMix::play_channel_array[MAXCHANNELS];
XWAVEHDR* WaveMix::block_array1[10];
XWAVEHDR* WaveMix::block_array2[10];
unsigned char* WaveMix::play_data[MAXCHANNELS];
volume_struct WaveMix::play_volume[MAXCHANNELS];
int WaveMix::play_counter = 0;

HANDLE WaveMix::Init()
{
	return ConfigureInit(nullptr);
}

HANDLE WaveMix::ConfigureInit(MIXCONFIG* lpConfig)
{
	MIXCONFIG mixConfig{};

	memset(&mixConfig, 0, sizeof(MIXCONFIG));
	unsigned int copySize = sizeof(MIXCONFIG);
	mixConfig.RegistryKey = nullptr;
	mixConfig.wSize = sizeof(MIXCONFIG);
	if (lpConfig)
	{
		if (lpConfig->wSize < sizeof(MIXCONFIG))
			copySize = lpConfig->wSize;
		memcpy(&mixConfig, lpConfig, copySize);
	}
	if (initialized_flag || Startup(GetModuleHandleA(nullptr)) != 0)
	{
		bool showDebugDialogs;
		if ((mixConfig.dwFlags & 0x100) != 0)
			showDebugDialogs = mixConfig.ShowDebugDialogs != 0;
		else
			showDebugDialogs = GetPrivateProfileIntA("general", "ShowDebugDialogs", 0, FileName) != 0;
		ShowDebugDialogs = showDebugDialogs;
		if (!waveOutGetNumDevs())
		{
			if (ShowDebugDialogs)
			{
				wsprintfA(string_buffer, "This system does not have a valid wave output device.");
				MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
			}
			return nullptr;
		}

		if (GetPrivateProfileIntA("general", "ShowDevices", 0, FileName))
			ShowWaveOutDevices();
		auto globals = static_cast<GLOBALS*>(LocalAlloc(LMEM_ZEROINIT, sizeof(GLOBALS)));
		Globals = globals;
		if (!globals)
			return nullptr;
		globals->CmixPtr = cmixit_ptr;
		globals->wMagic2 = 21554;
		globals->wMagic1 = 21554;
		globals->WaveBlockArray = nullptr;
		globals->SettingsDialogActiveFlag = 0;
		globals->DefaultVolume.L = 10;
		globals->DefaultVolume.R = 10;
		memset(globals->aChannel, 0xFFu, sizeof globals->aChannel);
		memmove(&globals->PCM, &gpFormat, sizeof(PCMWAVEFORMAT));
		if (!ReadConfigSettings(&mixConfig))
		{
			Globals->wMagic2 = 0;
			Globals->wMagic1 = 0;
			LocalFree(Globals);
			Globals = nullptr;
		}
		return Globals;
	}

	return nullptr;
}

int WaveMix::CloseSession(HANDLE hMixSession)
{
	Globals = SessionToGlobalDataPtr(hMixSession);
	if (!Globals)
		return 5;

	Activate(hMixSession, false);
	CloseChannel(hMixSession, 0, 1);
	memset(Globals, 0, sizeof(GLOBALS));
	Globals = nullptr;
	if (!hMixSession || !LocalFree(hMixSession))
		return 5;

	return 0;
}

int WaveMix::OpenChannel(HANDLE hMixSession, int iChannel, unsigned dwFlags)
{
	GLOBALS* globals = SessionToGlobalDataPtr(hMixSession);
	Globals = globals;
	if (!globals)
		return 5;
	if (dwFlags > 2)
		return 10;
	if (dwFlags == 2 && (iChannel > 16 || iChannel < 1))
		return 11;
	if (dwFlags == 0 && iChannel >= 16)
		return 11;

	if (dwFlags)
	{
		if (dwFlags == 1)
			iChannel = 16;

		for (auto index = iChannel - 1; index >= 0; --index)
		{
			if (globals->aChannel[index] == reinterpret_cast<CHANNELNODE*>(-1))
			{
				globals->aChannel[index] = nullptr;
				globals->ChannelVolume[index].L = globals->DefaultVolume.L;
				globals->ChannelVolume[index].R = globals->DefaultVolume.R;
				++globals->iChannels;
			}
		}
	}
	else
	{
		if (globals->aChannel[iChannel] != reinterpret_cast<CHANNELNODE*>(-1))
			return 4;
		globals->aChannel[iChannel] = nullptr;
		globals->ChannelVolume[iChannel].L = globals->DefaultVolume.L;
		globals->ChannelVolume[iChannel].R = globals->DefaultVolume.R;
		++globals->iChannels;
	}
	return 0;
}

int WaveMix::CloseChannel(HANDLE hMixSession, int iChannel, unsigned dwFlags)
{
	Globals = SessionToGlobalDataPtr(hMixSession);
	if (!Globals)
		return 5;

	int minChannel = iChannel, maxChannel;
	int result = FlushChannel(hMixSession, iChannel, dwFlags | 2);
	if (!result)
	{
		if ((dwFlags & 1) != 0)
		{
			minChannel = 0;
			maxChannel = 16;
		}
		else
		{
			maxChannel = iChannel + 1;
			if (iChannel >= maxChannel)
				return 0;
		}

		CHANNELNODE** channelPtr = &Globals->aChannel[minChannel];
		int index = maxChannel - minChannel;
		do
		{
			if (*channelPtr != reinterpret_cast<CHANNELNODE*>(-1))
			{
				*channelPtr = reinterpret_cast<CHANNELNODE*>(-1);
				--Globals->iChannels;
			}
			++channelPtr;
			--index;
		}
		while (index);
		return 0;
	}
	return result;
}

int WaveMix::FlushChannel(HANDLE hMixSession, int iChannel, unsigned dwFlags)
{
	int channelId;
	int lastChannel;

	int remixFlag = 0;
	auto globals = SessionToGlobalDataPtr(hMixSession);
	Globals = globals;
	if (!globals)
		return 5;

	if ((dwFlags & 1) != 0)
	{
		channelId = 0;
		lastChannel = 16;
	}
	else
	{
		channelId = iChannel;
		if (iChannel < 0 || iChannel >= 16)
			return 11;
		if (globals->aChannel[iChannel] == reinterpret_cast<CHANNELNODE*>(-1))
			return 5;
		lastChannel = iChannel + 1;
		if (iChannel >= lastChannel)
			return 0;
	}

	for (auto index = channelId; index < lastChannel; index++)
	{
		auto curChannel = globals->aChannel[index];
		if (curChannel != reinterpret_cast<CHANNELNODE*>(-1))
		{
			globals->aChannel[index] = nullptr;
			remixFlag |= curChannel != nullptr;
			while (curChannel)
			{
				auto tmp = curChannel->next;
				FreeChannelNode(curChannel);
				curChannel = tmp;
			}
		}
	}

	if (remixFlag && (dwFlags & 2) == 0 && globals->fActive)
	{
		globals->pfnRemix(MyWaveOutGetPosition(globals->hWaveOut, globals->fGoodGetPos), nullptr);
	}
	return 0;
}

MIXWAVE* WaveMix::OpenWave(HANDLE hMixSession, LPCSTR szWaveFilename, HINSTANCE hInst, unsigned dwFlags)
{
	_MMIOINFO pmmioinfo;
	_MMCKINFO pmmcki, pmmFmt;
	HWAVEOUT phwo;
	WAVEFORMATEX pwfx;
	HMMIO hMmio = nullptr;
	HGLOBAL hResData = nullptr;
	HPSTR lpData = nullptr;
	auto globals = SessionToGlobalDataPtr(hMixSession);
	pwfx.wFormatTag = globals->PCM.wf.wFormatTag;
	pwfx.nChannels = globals->PCM.wf.nChannels;
	pwfx.nSamplesPerSec = globals->PCM.wf.nSamplesPerSec;
	pwfx.nAvgBytesPerSec = globals->PCM.wf.nAvgBytesPerSec;
	Globals = globals;
	pwfx.nBlockAlign = globals->PCM.wf.nBlockAlign;
	pwfx.wBitsPerSample = globals->PCM.wBitsPerSample;
	pwfx.cbSize = 0;
	if (waveOutOpen(&phwo, 0xFFFFFFFF, &pwfx, 0, 0, 1u))
	{
		if (ShowDebugDialogs)
			MessageBoxA(nullptr, "The waveform device can't play this format.", "WavMix32", MB_ICONWARNING);
		return nullptr;
	}

	auto mixWave = static_cast<MIXWAVE*>(GlobalLock(GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, sizeof(MIXWAVE))));
	if (!mixWave)
	{
		if (ShowDebugDialogs)
			MessageBoxA(
				nullptr,
				"Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.",
				"WavMix32",
				MB_ICONINFORMATION);
		return nullptr;
	}

	do
	{
		if ((dwFlags & 2) != 0)
		{
			HRSRC hrsc = FindResourceA(hInst, szWaveFilename, "WAVE");
			if (!hrsc || (hResData = LoadResource(hInst, hrsc)) == nullptr)
			{
				if (HIWORD(szWaveFilename))
					wsprintfA(string_buffer, "Failed to open 'WAVE' resource '%s'.", szWaveFilename);
				else
					wsprintfA(string_buffer, "Failed to open 'WAVE' resource %u.", LOWORD(szWaveFilename));
				if (ShowDebugDialogs)
					MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONWARNING);
				break;
			}

			memset(&pmmioinfo, 0, sizeof pmmioinfo);
			pmmioinfo.pchBuffer = static_cast<HPSTR>(LockResource(hResData));
			if (!pmmioinfo.pchBuffer)
			{
				if (ShowDebugDialogs)
					MessageBoxA(nullptr, "Failed to lock 'WAVE' resource", "WavMix32", MB_ICONWARNING);
				FreeResource(hResData);
				hResData = nullptr;
				break;
			}

			pmmioinfo.cchBuffer = SizeofResource(hInst, hrsc);
			pmmioinfo.fccIOProc = FOURCC_MEM;
			pmmioinfo.adwInfo[0] = 0;
			hMmio = mmioOpenA(nullptr, &pmmioinfo, 0);
			if (!hMmio)
			{
				if (ShowDebugDialogs)
				{
					wsprintfA(string_buffer,
					          "Failed to open resource, mmioOpen error=%u.\nMay need to make sure resource is marked read-write",
					          pmmioinfo.wErrorRet);
					MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONWARNING);
				}
				break;
			}
		}
		else if ((dwFlags & 4) != 0)
		{
			memcpy(&pmmioinfo, szWaveFilename, sizeof pmmioinfo);
			hMmio = mmioOpenA(nullptr, &pmmioinfo, 0);
			if (!hMmio)
			{
				if (ShowDebugDialogs)
				{
					wsprintfA(string_buffer,
					          "Failed to open memory file, mmioOpen error=%u.\nMay need to make sure memory is read-write",
					          pmmioinfo.wErrorRet);
					MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONWARNING);
				}
				break;
			}
		}
		else
		{
			hMmio = mmioOpenA(const_cast<LPSTR>(szWaveFilename), nullptr, 0x10000u);
			if (!hMmio)
			{
				if (ShowDebugDialogs)
				{
					wsprintfA(string_buffer, "Failed to open wave file %s.", szWaveFilename);
					MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONWARNING);
				}
				break;
			}
		}

		pmmcki.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		if (mmioDescend(hMmio, &pmmcki, nullptr, MMIO_FINDRIFF))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "This is not a WAVE file.", "WavMix32", MB_ICONWARNING);
			break;
		}

		pmmFmt.ckid = mmioFOURCC('f', 'm', 't', ' ');
		if (mmioDescend(hMmio, &pmmFmt, &pmmcki, MMIO_FINDCHUNK))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "WAVE file is corrupted.", "WavMix32", MB_ICONWARNING);
			break;
		}
		if (mmioRead(hMmio, (HPSTR)mixWave, 16) != 16)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Failed to read format chunk.", "WavMix32", MB_ICONWARNING);
			break;
		}
		if (mixWave->pcm.wf.wFormatTag != 1)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "The file is not a PCM file.", "WavMix32", MB_ICONWARNING);
			break;
		}

		mmioAscend(hMmio, &pmmFmt, 0);
		pmmFmt.ckid = mmioFOURCC('d', 'a', 't', 'a');
		if (mmioDescend(hMmio, &pmmFmt, &pmmcki, MMIO_FINDCHUNK))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "WAVE file has no data chunk.", "WavMix32", MB_ICONWARNING);
			break;
		}
		auto dataSize = pmmFmt.cksize;
		if (!pmmFmt.cksize)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "The data chunk has no data.", "WavMix32", MB_ICONWARNING);
			break;
		}

		lpData = static_cast<HPSTR>(GlobalLock(GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, pmmFmt.cksize)));
		if (!lpData)
		{
			if (ShowDebugDialogs)
				MessageBoxA(
					nullptr,
					"Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.",
					"WavMix32",
					MB_ICONINFORMATION);
			break;
		}

		auto readCount = mmioRead(hMmio, lpData, dataSize);
		if (readCount != static_cast<LONG>(dataSize))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Failed to read data chunk.", "WavMix32", MB_ICONWARNING);
			break;
		}
		lpData = WaveFormatConvert(&Globals->PCM, &mixWave->pcm, lpData, &dataSize);
		if (!lpData)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Failed to convert wave format.", "WavMix32", MB_ICONWARNING);
			break;
		}
		mmioClose(hMmio, 0);
		if (hResData)
			FreeResource(hResData);
		mixWave->wh.dwBufferLength = dataSize;
		mixWave->wh.lpData = lpData;
		mixWave->wh.dwFlags = 0;
		mixWave->wh.dwLoops = 0;
		mixWave->wh.dwUser = 0;
		mixWave->wMagic = 21554;
		memmove(&mixWave->pcm, &Globals->PCM, sizeof(PCMWAVEFORMAT));

		if (HIWORD(szWaveFilename))
		{
			auto fileNameLength = lstrlenA(szWaveFilename);
			int copyOffset = fileNameLength > 15 ? fileNameLength - 15 : 0;
			lstrcpyA(mixWave->szWaveFilename, &szWaveFilename[copyOffset]);
		}
		else
		{
			wsprintfA(mixWave->szWaveFilename, "res#%u", LOWORD(szWaveFilename));
		}
		return mixWave;
	}
	while (false);

	if (hMmio)
		mmioClose(hMmio, 0);
	GlobalUnlock(GlobalHandle(mixWave));
	GlobalFree(GlobalHandle(mixWave));
	if (lpData)
	{
		GlobalUnlock(GlobalHandle(lpData));
		GlobalFree(GlobalHandle(lpData));
	}
	if (hResData)
		FreeResource(hResData);
	return nullptr;
}

int WaveMix::FreeWave(HANDLE hMixSession, MIXWAVE* lpMixWave)
{
	GLOBALS* globals = SessionToGlobalDataPtr(hMixSession);
	if (!globals)
		return 5;
	if (!IsValidLPMIXWAVE(lpMixWave))
		return 5;

	CHANNELNODE** channelPtr = globals->aChannel;
	for (auto index = 16; index; --index)
	{
		CHANNELNODE* channel = *channelPtr;
		if (channel != reinterpret_cast<CHANNELNODE*>(-1))
		{
			CHANNELNODE* prevChannel = nullptr;
			while (channel)
			{
				if (channel->lpMixWave == lpMixWave)
				{
					if (prevChannel)
					{
						prevChannel->next = channel->next;
						FreeChannelNode(channel);
						channel = prevChannel->next;
					}
					else
					{
						channel = channel->next;
						FreeChannelNode(channel);
						*channelPtr = channel;
					}
				}
				else
				{
					prevChannel = channel;
					channel = channel->next;
				}
			}
		}
		++channelPtr;
	}

	if (lpMixWave->wh.lpData)
	{
		GlobalUnlock(GlobalHandle(lpMixWave->wh.lpData));
		GlobalFree(GlobalHandle(lpMixWave->wh.lpData));
	}
	lpMixWave->wMagic = 0;
	GlobalUnlock(GlobalHandle(lpMixWave));
	GlobalFree(GlobalHandle(lpMixWave));
	return 0;
}

int WaveMix::Activate(HANDLE hMixSession, bool fActivate)
{
	GLOBALS* globals = SessionToGlobalDataPtr(hMixSession);
	Globals = globals;
	if (!globals)
		return 5;
	if (fActivate)
	{
		if (GlobalsActive)
			return GlobalsActive != globals ? 4 : 0;
		if (globals->SettingsDialogActiveFlag)
			return 12;
		GlobalsActive = globals;
		sndPlaySoundA(nullptr, 0);
		auto result = GetWaveDevice();
		if (result)
		{
			GlobalsActive = nullptr;
			return result;
		}
		Globals->fActive = 1;
		SetWaveOutPosition(Globals->dwCurrentSample);
		XWAVEHDR* xHDR;
		do
			xHDR = GetWaveBlock();
		while (MixerPlay(xHDR, 1));
	}
	else
	{
		if (globals->fActive)
		{
			Globals->dwCurrentSample = MyWaveOutGetPosition(globals->hWaveOut, globals->fGoodGetPos);
		}
		ReleaseWaveDevice(globals);
		Globals->fActive = 0;
		if (Globals == GlobalsActive)
			GlobalsActive = nullptr;
	}
	return 0;
}

void WaveMix::Pump(bool performCleanup)
{
	Globals = GlobalsActive;
	if (GlobalsActive)
	{
		auto xHDR = play_queue.first;
		while (xHDR)
		{
			if ((xHDR->wh.dwFlags & 1) != 0)
			{
				RemoveFromPlayingQueue(xHDR);
				xHDR->fAvailable = 1;
				xHDR = play_queue.first;
			}
			else
				xHDR = xHDR->QNext;
		}
		if (performCleanup)
			FreePlayedBlocks();
		while (MixerPlay(GetWaveBlock(), 1));
	}
}

int WaveMix::Play(MIXPLAYPARAMS* lpMixPlayParams)
{
	++play_counter;
	auto result = 12;
	auto remixFlag = 0;

	do
	{
		if (play_counter > 1)
			break;

		if (!lpMixPlayParams)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "NULL parameters pointer passed to WaveMixPlay!", "WavMix32", MB_ICONWARNING);
			result = 5;
			break;
		}

		auto globals = SessionToGlobalDataPtr(lpMixPlayParams->hMixSession);
		Globals = globals;
		if (!globals)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Invalid session handle passed to WaveMixPlay", "WavMix32", MB_ICONWARNING);
			result = 5;
			break;
		}

		if (!IsValidLPMIXWAVE(lpMixPlayParams->lpMixWave))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Invalid or NULL wave pointer passed to WaveMixPlay!", "WavMix32", MB_ICONWARNING);
			break;
		}

		if (!HasCurrentOutputFormat(lpMixPlayParams->lpMixWave))
		{
			wsprintfA(
				string_buffer,
				"The LPMIXWAVE 0x%lx is not in the current output format, close the wave and reopen it.",
				lpMixPlayParams->lpMixWave);
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONWARNING);
			result = 8;
			break;
		}
		if (!globals->fActive)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Wave device not allocated, call WaveMixActivate(hMixSession,TRUE)", "WavMix32",
				            MB_ICONWARNING);
			result = 4;
			break;
		}
		if (!globals->iChannels)
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "You must open a channel before you can play a wave!", "WavMix32", MB_ICONWARNING);
			result = 5;
			break;
		}

		int iChannel;
		if ((lpMixPlayParams->dwFlags & WMIX_USELRUCHANNEL) != 0)
		{
			iChannel = 0;
			for (auto channelIndex = 0; channelIndex < 16; ++channelIndex)
			{
				if (globals->aChannel[channelIndex] != reinterpret_cast<CHANNELNODE*>(-1))
				{
					if (!globals->aChannel[iChannel])
						break;
					if (channelIndex != iChannel && globals->MRUChannel[channelIndex] < globals->MRUChannel[iChannel])
						iChannel = channelIndex;
				}
			}
			lpMixPlayParams->iChannel = iChannel;
		}
		else
		{
			iChannel = lpMixPlayParams->iChannel;
		}
		++globals->dwMRU;
		globals->MRUChannel[iChannel] = globals->dwMRU;
		if (globals->aChannel[iChannel] == reinterpret_cast<CHANNELNODE*>(-1))
		{
			result = 5;
			break;
		}

		auto channel = GetChannelNode();
		if (!channel)
			break;

		memcpy(&channel->PlayParams, lpMixPlayParams, sizeof channel->PlayParams);
		channel->lpMixWave = channel->PlayParams.lpMixWave;
		channel->dwNumSamples = channel->PlayParams.lpMixWave->wh.dwBufferLength;
		auto lpData = (uint8_t*)channel->PlayParams.lpMixWave->wh.lpData;
		channel->lpPos = lpData;
		channel->lpEnd = &lpData[channel->dwNumSamples - globals->PCM.wf.nBlockAlign];
		channel->PlayParams.iChannel = iChannel;
		if (globals->pWaitList)
		{
			channel->next = globals->pWaitList->next;
			globals->pWaitList->next = channel;
			globals->pWaitList = channel;
		}
		else
		{
			globals->pWaitList = channel;
			channel->next = channel;
		}

		if ((channel->PlayParams.dwFlags & WMIX_WAIT) != 0)
		{
			result = 0;
			break;
		}

		ResetWavePosIfNoChannelData();
		auto globals2 = Globals;
		unsigned wavePosition;
		if (Globals->pfnRemix == ResetRemix)
		{
			wavePosition = MyWaveOutGetPosition(Globals->hWaveOut, Globals->fGoodGetPos);
			globals2 = Globals;
		}
		else
		{
			wavePosition = Globals->dwCurrentSample;
		}

		while (globals2->pWaitList)
		{
			auto curChannel = globals2->pWaitList->next;
			if (globals2->pWaitList->next == globals2->pWaitList)
				globals2->pWaitList = nullptr;
			else
				globals2->pWaitList->next = curChannel->next;

			iChannel = curChannel->PlayParams.iChannel;
			curChannel->next = nullptr;
			if ((curChannel->PlayParams.dwFlags & WMIX_CustomVolume) != 0)
			{
				curChannel->Volume.L = curChannel->PlayParams.Volume.L;
				curChannel->Volume.R = curChannel->PlayParams.Volume.R;
			}
			else
			{
				curChannel->Volume.L = globals2->ChannelVolume[iChannel].L;
				curChannel->Volume.R = globals2->ChannelVolume[iChannel].R;
			}
			if (curChannel->Volume.L > 10u)
				curChannel->Volume.L = 10;
			if (curChannel->Volume.R > 10u)
				curChannel->Volume.R = 10;

			if ((curChannel->PlayParams.dwFlags & WMIX_CLEARQUEUE) != 0)
			{
				for (auto tmpCh = globals2->aChannel[iChannel]; tmpCh;)
				{
					auto nextChannel = tmpCh->next;
					FreeChannelNode(tmpCh);
					tmpCh = nextChannel;
				}
				globals2->aChannel[iChannel] = curChannel;
				if (play_queue.first != nullptr)
					remixFlag = 1;
				if ((curChannel->PlayParams.dwFlags & WMIX_HIPRIORITY) != 0)
					curChannel->dwStartPos = wavePosition;
				else
					curChannel->dwStartPos = globals2->dwCurrentSample;
			}
			else
			{
				DWORD dwStartPos;
				if (globals2->aChannel[iChannel])
				{
					auto tmpCh = globals2->aChannel[iChannel];
					while (tmpCh->next)
						tmpCh = tmpCh->next;
					tmpCh->next = curChannel;
					dwStartPos = tmpCh->dwEndPos;

					if ((curChannel->PlayParams.dwFlags & WMIX_HIPRIORITY) != 0)
					{
						if (dwStartPos <= wavePosition)
							dwStartPos = wavePosition;
					}
					else if (globals2->dwCurrentSample > dwStartPos)
					{
						dwStartPos = globals2->dwCurrentSample;
					}
				}
				else
				{
					dwStartPos = wavePosition;
					globals2->aChannel[iChannel] = curChannel;
					if ((curChannel->PlayParams.dwFlags & WMIX_HIPRIORITY) == 0)
						dwStartPos = globals2->dwCurrentSample;
				}
				curChannel->dwStartPos = dwStartPos;
				if (globals2->dwCurrentSample > curChannel->dwStartPos)
					remixFlag = 1;
			}
			if (curChannel->PlayParams.wLoops == 0xFFFF)
				curChannel->dwEndPos = -1;
			else
				curChannel->dwEndPos = curChannel->dwStartPos + curChannel->dwNumSamples * (curChannel->PlayParams.
					wLoops + 1) - globals2->PCM.wf.nBlockAlign;
		}

		if (!remixFlag || !globals2->pfnRemix(wavePosition, nullptr))
		{
			int pauseFlag;
			if (play_queue.first || globals2->PauseBlocks <= 0)
			{
				pauseFlag = 0;
			}
			else
			{
				waveOutPause(globals2->hWaveOut);
				pauseFlag = 1;
			}

			auto pauseCounter = 0;
			while (MixerPlay(GetWaveBlock(), 1))
			{
				if (pauseFlag)
				{
					if (++pauseCounter >= Globals->PauseBlocks)
					{
						waveOutRestart(Globals->hWaveOut);
						pauseFlag = 0;
					}
				}
			}
			if (pauseFlag)
				waveOutRestart(Globals->hWaveOut);
		}

		result = 0;
	}
	while (false);

	--play_counter;
	return result;
}

GLOBALS* WaveMix::SessionToGlobalDataPtr(HANDLE hMixSession)
{
	auto globals = static_cast<GLOBALS*>(hMixSession);
	if (hMixSession && globals->wMagic1 == 21554 && globals->wMagic2 == 21554)
		return globals;
	MessageBeep(0xFFFFFFFF);
	wsprintfA(string_buffer, "Invalid session handle 0x%04X passed to WaveMix API", hMixSession);
	MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONWARNING);
	return nullptr;
}

int WaveMix::Startup(HMODULE hModule)
{
	WNDCLASSA WndClass;

	if (initialized_flag)
		return 1;
	if (!SetIniFileName(hModule))
		return 0;
	InitVolumeTable();
	debug_flag = GetPrivateProfileIntA("general", "debug", 0, FileName);
	cmixit_ptr = cmixit;
	HModule = hModule;

	WndClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
	WndClass.hIcon = nullptr;
	WndClass.lpszMenuName = nullptr;
	WndClass.lpszClassName = "WavMix32";
	WndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(1));
	WndClass.hInstance = HModule;
	WndClass.style = 0;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbWndExtra = 0;
	WndClass.cbClsExtra = 0;
	if (!RegisterClassA(&WndClass))
		return 0;
	InitChannelNodes();
	initialized_flag = 1;
	return 1;
}

int WaveMix::SetIniFileName(HMODULE hModule)
{
	int result = GetModuleFileNameA(hModule, FileName, 0x104u);
	if (result)
	{
		char* i;
		for (i = &FileName[result]; *i != '\\'; --i)
		{
		}
		*i = 0;
		lstrcpyA(i, "\\WAVEMIX.INF");
		result = 1;
	}
	return result;
}

void WaveMix::InitChannelNodes()
{
	CHANNELNODE* channelPtr = channel_nodes;
	do
	{
		channelPtr->next = channelPtr + 1;
		++channelPtr;
	}
	while (channelPtr < &channel_nodes[MAXQUEUEDWAVES - 2]);
	channel_nodes[MAXQUEUEDWAVES - 1].next = nullptr;
	free_channel_nodes = channel_nodes;
}

void WaveMix::InitVolumeTable()
{
	int index3Sub = 0;
	for (auto volume = 0; volume < 11; volume++)
	{
		auto tablePtr = &volume_table[volume][0];
		for (auto divSmth = index3Sub, sample = 0; sample < 256; ++sample)
		{
			tablePtr[sample] = static_cast<unsigned char>(divSmth / 10 + 128);
			divSmth += volume;
		}
		index3Sub -= 128;
	}
}

void WaveMix::ShowWaveOutDevices()
{
	tagWAVEOUTCAPSA pwoc{};

	auto deviceCount = waveOutGetNumDevs();
	if (deviceCount)
	{
		wsprintfA(string_buffer, "%d waveOut Devices have been detected on your system.", deviceCount);
		MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
		for (auto uDeviceID = 0u; uDeviceID < deviceCount; ++uDeviceID)
		{
			if (!waveOutGetDevCapsA(uDeviceID, &pwoc, 0x34u) && RemoveInvalidIniNameCharacters(pwoc.szPname))
				wsprintfA(
					string_buffer,
					"Device %i: %s\n\tVersion %u.%u",
					uDeviceID,
					pwoc.szPname,
					HIBYTE(pwoc.vDriverVersion),
					LOBYTE(pwoc.vDriverVersion));
			else
				wsprintfA(string_buffer, "waveOutGetDevCaps failed (err %u) for device %d", 1, uDeviceID);
			MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
		}
	}
}

int WaveMix::RemoveInvalidIniNameCharacters(char* lpString)
{
	auto stringPtr = lpString;
	if (!lpString || !*lpString)
		return 0;
	do
	{
		if (!isalnum(*stringPtr) && !isspace(*stringPtr))
			break;
		++stringPtr;
	}
	while (*stringPtr);

	do
		*stringPtr-- = 0;
	while (stringPtr >= lpString && isspace(*stringPtr));
	return lstrlenA(lpString);
}

int WaveMix::ReadConfigSettings(MIXCONFIG* lpConfig)
{
	auto waveDeviceCount = waveOutGetNumDevs();
	if (!waveDeviceCount)
		return 0;
	if ((lpConfig->dwFlags & 0x400) != 0 && ReadRegistryForAppSpecificConfigs(lpConfig))
	{
		Configure(Globals, nullptr, lpConfig, nullptr, 0);
		return 1;
	}

	UINT deviceId;
	if (static_cast<char>(lpConfig->dwFlags) >= 0)
	{
		deviceId = GetPrivateProfileIntA("general", "WaveOutDevice", 0, FileName);
	}
	else
	{
		deviceId = lpConfig->wDeviceID;
	}
	Globals->wDeviceID = deviceId;
	if (Globals->wDeviceID >= waveDeviceCount)
		Globals->wDeviceID = 0;

	if (waveOutGetDevCapsA(Globals->wDeviceID, &Globals->WaveoutCaps, 0x34u)
		|| !RemoveInvalidIniNameCharacters(Globals->WaveoutCaps.szPname))
	{
		lstrcpyA(Globals->WaveoutCaps.szPname, "Unkown Device");
	}

	if (!ReadRegistryToGetMachineSpecificInfSection(Globals->wDeviceID, Globals->szDevicePName, 96))
	{
		lstrcpyA(Globals->szDevicePName, GetOperatingSystemPrefix());
		lstrcatA(Globals->szDevicePName, Globals->WaveoutCaps.szPname);
	}

	auto remix = GetPrivateProfileIntA("default", "Remix", 1, FileName);
	auto goodWavePos = GetPrivateProfileIntA("default", "GoodWavePos", DefaultGoodWavePos(Globals->wDeviceID),
	                                         FileName);
	auto waveBlocks = GetPrivateProfileIntA("default", "WaveBlocks", 3, FileName);
	auto samplesPerSec = GetPrivateProfileIntA("default", "SamplesPerSec", 11, FileName);
	if ((Globals->WaveoutCaps.dwSupport & 0x10) != 0)
	{
		if (!ShowDebugDialogs)
			return 0;
		wsprintfA(string_buffer,
		          "%s is a syncronous (blocking) wave output device.  This will not permit audio to play while other applications are running.",
		          Globals->WaveoutCaps.szPname);
		MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
		return 0;
	}
	if (GetPrivateProfileIntA("not compatible", Globals->szDevicePName, 0, FileName))
	{
		if (!ShowDebugDialogs)
			return 0;
		wsprintfA(string_buffer, "%s is not compatible with the realtime wavemixer.", Globals->szDevicePName);
		MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
		return 0;
	}

	Globals->pfnRemix = GetPrivateProfileIntA(Globals->szDevicePName, "Remix", remix, FileName) != 2
		                    ? ResetRemix
		                    : NoResetRemix;
	Globals->fGoodGetPos = GetPrivateProfileIntA(Globals->szDevicePName, "GoodWavePos", goodWavePos, FileName) != 0;
	Globals->WaveBlockCount = GetPrivateProfileIntA(Globals->szDevicePName, "WaveBlocks", waveBlocks, FileName);
	if (Globals->WaveBlockCount < 2)
		Globals->WaveBlockCount = 2;
	else if (Globals->WaveBlockCount > 10)
		Globals->WaveBlockCount = 10;

	Globals->PauseBlocks = GetPrivateProfileIntA(Globals->szDevicePName, "PauseBlocks",
	                                             DefaultPauseBlocks(Globals->WaveBlockCount), FileName);
	if (Globals->PauseBlocks >= 0)
	{
		if (Globals->PauseBlocks > Globals->WaveBlockCount)
			Globals->PauseBlocks = Globals->WaveBlockCount;
	}
	else
	{
		Globals->PauseBlocks = 0;
	}

	auto samplesPerSec2 = GetPrivateProfileIntA(Globals->szDevicePName, "SamplesPerSec", samplesPerSec, FileName);
	auto channels = GetPrivateProfileIntA(Globals->szDevicePName, "Channels", 1, FileName);
	Globals->PCM.wf.nChannels = channels;
	if (channels)
	{
		if (channels > 2u)
			Globals->PCM.wf.nChannels = 2;
	}
	else
	{
		Globals->PCM.wf.nChannels = 1;
	}
	DWORD nAvgBytesPerSec;
	if (samplesPerSec2 == 22)
	{
		Globals->PCM.wf.nSamplesPerSec = 22050;
		nAvgBytesPerSec = 22050 * Globals->PCM.wf.nChannels;
	}
	else if (samplesPerSec2 == 44)
	{
		Globals->PCM.wf.nSamplesPerSec = 44100;
		nAvgBytesPerSec = 44100 * Globals->PCM.wf.nChannels;
	}
	else
	{
		nAvgBytesPerSec = 11025 * Globals->PCM.wf.nChannels;
	}
	Globals->PCM.wf.nAvgBytesPerSec = nAvgBytesPerSec;
	auto waveBlockLen2 = GetPrivateProfileIntA("default", "WaveBlockLen", 0, FileName);
	Globals->dwWaveBlockLen = FigureOutDMABufferSize(waveBlockLen2, &Globals->PCM);
	Configure(Globals, nullptr, lpConfig, nullptr, 0);
	return 1;
}

int WaveMix::ReadRegistryForAppSpecificConfigs(MIXCONFIG* lpConfig)
{
	HKEY phkResult;
	CHAR SubKey[52];

	DWORD dwFlags = lpConfig->dwFlags;
	if ((dwFlags & 0x400) == 0 || !lpConfig->RegistryKey)
		return 0;
	if ((dwFlags & 0x80u) == 0)
	{
		lpConfig->wDeviceID = Globals->wDeviceID;
	}
	else if (lpConfig->wDeviceID >= waveOutGetNumDevs())
	{
		lpConfig->wDeviceID = 0;
	}
	wsprintfA(SubKey, "WaveMix\\Device%u", lpConfig->wDeviceID);
	if (RegOpenKeyA(lpConfig->RegistryKey, SubKey, &phkResult))
		return 0;
	if ((dwFlags & 1) == 0)
		lpConfig->wChannels = ReadRegistryInt(phkResult, "Channels", 1);
	if ((dwFlags & 2) == 0)
		lpConfig->wSamplingRate = ReadRegistryInt(phkResult, "SamplesPerSec", 11);
	if ((dwFlags & 4) == 0)
		lpConfig->WaveBlockCount = static_cast<short>(ReadRegistryInt(phkResult, "WaveBlocks", 3));
	if ((dwFlags & 8) == 0)
		lpConfig->WaveBlockLen = static_cast<short>(ReadRegistryInt(phkResult, "WaveBlockLen", 0));
	lpConfig->CmixPtrDefaultFlag = 1;
	if ((dwFlags & 0x20) == 0)
		lpConfig->ResetMixDefaultFlag = static_cast<uint16_t>(ReadRegistryInt(phkResult, "Remix", 1)) != 2;
	if ((dwFlags & 0x40) == 0)
	{
		int defaultGoodWavePos = DefaultGoodWavePos(lpConfig->wDeviceID);
		lpConfig->GoodWavePos = static_cast<uint16_t>(ReadRegistryInt(
			phkResult, "GoodWavePos", defaultGoodWavePos)) != 0;
	}
	if ((dwFlags & 0x100) == 0)
		lpConfig->ShowDebugDialogs = static_cast<short>(ReadRegistryInt(phkResult, "ShowDebugDialogs", 0));
	if ((dwFlags & 0x200) == 0)
	{
		int defaultPauseBlocks = DefaultPauseBlocks(static_cast<uint16_t>(lpConfig->WaveBlockCount));
		lpConfig->PauseBlocks = static_cast<short>(ReadRegistryInt(phkResult, "PauseBlocks", defaultPauseBlocks));
	}
	lpConfig->dwFlags = 1023;
	waveOutGetDevCapsA(lpConfig->wDeviceID, &Globals->WaveoutCaps, 0x34u);
	RegCloseKey(phkResult);
	return 1;
}

int WaveMix::ReadRegistryInt(HKEY hKey, LPCSTR lpSubKey, int defaultValue)
{
	int result;
	LONG cbData = 10;
	char Data[12];

	if (!hKey || RegQueryValueA(hKey, lpSubKey, Data, &cbData))
		result = defaultValue;
	else
		result = atol(Data);
	return result;
}

int WaveMix::DefaultGoodWavePos(unsigned uDeviceID)
{
	int result;
	struct tagWAVEOUTCAPSA pwoc{};

	auto deviceCount = waveOutGetNumDevs();
	if (uDeviceID > deviceCount || (uDeviceID & 0x80000000) != 0 ||
		!deviceCount || waveOutGetDevCapsA(uDeviceID, &pwoc, 0x34u))
	{
		result = 0;
	}
	else
	{
		result = LOBYTE(pwoc.dwSupport) >> 5 & 1;
	}
	return result;
}

#pragma warning (disable : 4996)/*Original uses GetVersion*/
int WaveMix::DefaultPauseBlocks(int waveBlocks)
{
	int result;
	if (GetVersion() < 0x80000000 || static_cast<uint8_t>(GetVersion()) < 4u)
		result = waveBlocks;
	else
		result = 0;
	return result;
}
#pragma warning (default : 4996)

int WaveMix::Configure(GLOBALS* hMixSession, HWND hWndParent, MIXCONFIG* lpConfig, int* flag1Ptr, int saveConfigFlag)
{
	MIXCONFIG mixConfigLocal{};
	tagWAVEOUTCAPSA pwoc{};

	auto mixConfig = lpConfig;
	auto someFlag1 = 0;
	auto globals = SessionToGlobalDataPtr(hMixSession);
	Globals = globals;
	if (!globals)
		return 5;
	if (globals->fActive)
		return 4;
	if (globals->SettingsDialogActiveFlag)
		return 12;
	FlushChannel(hMixSession, -1, 1u);

	if (!mixConfig)
	{
		mixConfigLocal.wSize = sizeof(MIXCONFIG);
		mixConfigLocal.dwFlags = 1023;
		GetConfig(static_cast<GLOBALS*>(hMixSession), &mixConfigLocal);
		auto dialog = MakeSettingsDlgTemplate();
		if (!dialog)
			return 1;
		Globals->SettingsDialogActiveFlag = 1;
		auto dlgResult = DialogBoxIndirectParamA(HModule, &dialog->Dialog, hWndParent, SettingsDlgProc,
		                                         (LPARAM)&mixConfigLocal);
		DestroySettingsDlgTemplate(dialog);
		Globals->SettingsDialogActiveFlag = 0;
		if (dlgResult != 1)
			return 1;
		if (Globals->dwWaveBlockLen == mixConfigLocal.WaveBlockLen)
			mixConfigLocal.dwFlags &= 0xFFFFFFF7;

		mixConfig = &mixConfigLocal;
	}
	if (!mixConfig->dwFlags)
		return 1;

	globals = SessionToGlobalDataPtr(hMixSession);
	Globals = globals;
	if (!globals)
		return 5;

	auto dwFlags = mixConfig->dwFlags;
	if ((dwFlags & 0x100) != 0)
		ShowDebugDialogs = mixConfig->ShowDebugDialogs != 0;
	if ((dwFlags & 0x80u) != 0)
	{
		if (mixConfig->wDeviceID != globals->wDeviceID)
		{
			auto result = waveOutGetDevCapsA(mixConfig->wDeviceID, &pwoc, 0x34u);
			if (result)
				return result;
			memcpy(&Globals->WaveoutCaps, &pwoc, sizeof Globals->WaveoutCaps);
			Globals->wDeviceID = mixConfig->wDeviceID;
			if (Globals->WaveoutCaps.wChannels == 1 && Globals->PCM.wf.nChannels == 2)
			{
				Globals->PCM.wf.nChannels = 1;
				someFlag1 = 1;
				Globals->PCM.wf.nBlockAlign = 1;
				Globals->PCM.wf.nAvgBytesPerSec = Globals->PCM.wf.nSamplesPerSec;
			}
			lstrcpyA(globals->szDevicePName, pwoc.szPname);
			if (!RemoveInvalidIniNameCharacters(Globals->szDevicePName))
				lstrcpyA(Globals->szDevicePName, "Unkown Device");
			if (!ReadRegistryToGetMachineSpecificInfSection(Globals->wDeviceID, Globals->szDevicePName, 96))
			{
				lstrcpyA(Globals->szDevicePName, GetOperatingSystemPrefix());
				lstrcatA(Globals->szDevicePName, Globals->WaveoutCaps.szPname);
			}
		}
	}

	if ((dwFlags & 1) != 0)
	{
		if (mixConfig->wChannels <= 1u)
		{
			if (globals->PCM.wf.nChannels == 2)
				someFlag1 = 1;
			globals->PCM.wf.nChannels = 1;
			globals->PCM.wf.nBlockAlign = 1;
			globals->PCM.wf.nAvgBytesPerSec = globals->PCM.wf.nSamplesPerSec;
		}
		if (globals->WaveoutCaps.wChannels > 1u)
		{
			if (globals->PCM.wf.nChannels == 1)
				someFlag1 = 1;
			globals->PCM.wf.nChannels = 2;
			globals->PCM.wf.nBlockAlign = 2;
			globals->PCM.wf.nAvgBytesPerSec = 2 * globals->PCM.wf.nSamplesPerSec;
		}
	}

	if ((dwFlags & 2) != 0)
	{
		auto nAvgBytesPerSec = 0u;
		if (mixConfig->wSamplingRate == 22)
		{
			if (globals->PCM.wf.nSamplesPerSec != 22050)
				someFlag1 = 1;
			globals->PCM.wf.nSamplesPerSec = 22050;
			nAvgBytesPerSec = 22050 * globals->PCM.wf.nChannels;
		}
		else
		{
			if (mixConfig->wSamplingRate == 44)
			{
				if (globals->PCM.wf.nSamplesPerSec != 44100)
					someFlag1 = 1;
				globals->PCM.wf.nSamplesPerSec = 44100;
				nAvgBytesPerSec = 44100 * globals->PCM.wf.nChannels;
			}
			else
			{
				if (globals->PCM.wf.nSamplesPerSec != 11025)
					someFlag1 = 1;
				globals->PCM.wf.nSamplesPerSec = 11025;
				nAvgBytesPerSec = 11025 * globals->PCM.wf.nChannels;
			}
		}
		globals->PCM.wf.nAvgBytesPerSec = nAvgBytesPerSec;
	}

	if ((dwFlags & 4) != 0)
	{
		auto v24 = 2;
		globals->WaveBlockCount = mixConfig->WaveBlockCount;
		if (mixConfig->WaveBlockCount < 2)
			globals->WaveBlockCount = 2;
		else if (mixConfig->WaveBlockCount > 10)
			globals->WaveBlockCount = 10;
		if (globals->PauseBlocks >= 0)
		{
			if (globals->PauseBlocks > globals->WaveBlockCount)
				globals->PauseBlocks = globals->WaveBlockCount;
		}
		else
		{
			globals->PauseBlocks = 0;
		}
	}

	if (someFlag1)
		globals->dwWaveBlockLen = FigureOutDMABufferSize(0, &globals->PCM);

	if ((dwFlags & 8) != 0)
	{
		if (mixConfig->WaveBlockLen)
			globals->dwWaveBlockLen = mixConfig->WaveBlockLen;
	}

	if (globals->dwWaveBlockLen < 344)
		globals->dwWaveBlockLen = 344;
	else if (globals->dwWaveBlockLen > 11025)
		globals->dwWaveBlockLen = 11025;

	auto dwFlags2 = dwFlags;
	if ((dwFlags & 0x10) != 0)
		globals->CmixPtr = cmixit;

	if ((dwFlags2 & 0x20) != 0)
	{
		globals->pfnRemix = !mixConfig->ResetMixDefaultFlag ? NoResetRemix : ResetRemix;
	}

	if ((dwFlags2 & 0x40) != 0)
		globals->fGoodGetPos = mixConfig->GoodWavePos != 0;

	if ((dwFlags2 & 0x200) != 0)
	{
		globals->PauseBlocks = mixConfig->PauseBlocks;
		if (mixConfig->PauseBlocks > globals->WaveBlockCount)
			globals->PauseBlocks = globals->WaveBlockCount;
	}

	GetConfig(hMixSession, mixConfig);
	if (flag1Ptr)
		*flag1Ptr = someFlag1;
	if (saveConfigFlag)
		SaveConfigSettings(dwFlags);
	if (ShowDebugDialogs)
		ShowCurrentSettings();
	return 0;
}

int WaveMix::GetConfig(HANDLE hMixSession, MIXCONFIG* lpConfig)
{
	GLOBALS* globals = SessionToGlobalDataPtr(static_cast<GLOBALS*>(hMixSession));
	Globals = globals;
	if (!globals)
		return 5;
	if (!lpConfig)
		return 11;

	DWORD dwFlags = lpConfig->dwFlags;
	if ((dwFlags & 1) != 0)
		lpConfig->wChannels = globals->PCM.wf.nChannels;
	if ((dwFlags & 2) != 0)
		lpConfig->wSamplingRate = static_cast<WORD>(11 * (globals->PCM.wf.nSamplesPerSec / 0x2B11));
	if ((dwFlags & 4) != 0)
		lpConfig->WaveBlockCount = globals->WaveBlockCount;
	if ((dwFlags & 8) != 0)
		lpConfig->WaveBlockLen = static_cast<WORD>(globals->dwWaveBlockLen);
	if ((dwFlags & 0x10) != 0)
		lpConfig->CmixPtrDefaultFlag = globals->CmixPtr == cmixit;
	if ((dwFlags & 0x20) != 0)
		lpConfig->ResetMixDefaultFlag = globals->pfnRemix == ResetRemix;
	if ((dwFlags & 0x40) != 0)
		lpConfig->GoodWavePos = globals->fGoodGetPos;
	if ((dwFlags & 0x80u) != 0)
		lpConfig->wDeviceID = globals->wDeviceID;
	if ((dwFlags & 0x100) != 0)
		lpConfig->ShowDebugDialogs = ShowDebugDialogs != 0;
	if ((dwFlags & 0x200) != 0)
		lpConfig->PauseBlocks = globals->PauseBlocks;
	return 0;
}

unsigned WaveMix::MyWaveOutGetPosition(HWAVEOUT hWaveOut, int fGoodGetPos)
{
	MMTIME pmmt{};

	if (!fGoodGetPos)
		return (timeGetTime() - Globals->dwBaseTime) * Globals->PCM.wf.nAvgBytesPerSec / 0x3E8 & 0xFFFFFFF8;
	pmmt.wType = TIME_BYTES;
	waveOutGetPosition(hWaveOut, &pmmt, sizeof(MMTIME));
	return Globals->pfnSampleAdjust(pmmt.u.ms, Globals->dwWaveOutPos);
}

void WaveMix::FreeChannelNode(CHANNELNODE* channel)
{
	if (channel)
	{
		channel->next = free_channel_nodes;
		free_channel_nodes = channel;
	}
}

int WaveMix::ResetRemix(DWORD dwRemixSamplePos, CHANNELNODE* channel)
{
	Globals->dwCurrentSample = dwRemixSamplePos;
	DestroyPlayQueue();
	SwapWaveBlocks();

	while (true)
	{
		auto block = GetWaveBlock();
		if (!block)
			break;
		if (!MixerPlay(block, 0))
		{
			block->fAvailable = 1;
			break;
		}
		AddToPlayingQueue(block);
	}

	auto dwCurrentSamplePrev = Globals->dwCurrentSample;
	MyWaveOutReset(Globals->hWaveOut);
	Globals->dwCurrentSample = dwCurrentSamplePrev;
	auto pauseFlag = 0;
	if (Globals->PauseBlocks > 0)
	{
		waveOutPause(Globals->hWaveOut);
		pauseFlag = 1;
	}

	auto pauseCount = 0;
	for (auto xHDR = play_queue.first; xHDR; xHDR = xHDR->QNext)
	{
		if (waveOutWrite(Globals->hWaveOut, &xHDR->wh, sizeof(WAVEHDR)))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Failed to write block to device", "WavMix32", MB_ICONWARNING);
			xHDR->fAvailable = 1;
			RemoveFromPlayingQueue(xHDR);
		}
		if (pauseFlag)
		{
			if (++pauseCount >= Globals->PauseBlocks)
			{
				waveOutRestart(Globals->hWaveOut);
				pauseFlag = 0;
			}
		}
	}
	if (pauseFlag)
		waveOutRestart(Globals->hWaveOut);
	return 1;
}

XWAVEHDR* WaveMix::RemoveFromPlayingQueue(XWAVEHDR* lpXWH)
{
	if (!play_queue.first)
		return nullptr;

	if (lpXWH != play_queue.first)
	{
		XWAVEHDR* prev = play_queue.first;
		XWAVEHDR* current = play_queue.first->QNext;
		while (current)
		{
			if (current == lpXWH)
				break;
			prev = current;
			current = current->QNext;
		}
		if (!current)
			return nullptr;

		prev->QNext = current->QNext;
		if (current == play_queue.last)
			play_queue.last = prev;
	}
	else
	{
		play_queue.first = lpXWH->QNext;
		if (!play_queue.first)
			play_queue.last = nullptr;
	}

	lpXWH->QNext = nullptr;
	return lpXWH;
}

void WaveMix::DestroyPlayQueue()
{
	while (play_queue.first)
	{
		play_queue.first->fAvailable = 1;
		RemoveFromPlayingQueue(play_queue.first);
	}
}

void WaveMix::SwapWaveBlocks()
{
	if (Globals->WaveBlockArray == block_array1)
		Globals->WaveBlockArray = block_array2;
	else
		Globals->WaveBlockArray = block_array1;
}

XWAVEHDR* WaveMix::GetWaveBlock()
{
	int index = 0;
	for (; index < Globals->WaveBlockCount; index++)
		if (Globals->WaveBlockArray[index]->fAvailable)
			break;
	if (index >= Globals->WaveBlockCount)
		return nullptr;

	XWAVEHDR* result = Globals->WaveBlockArray[index];
	result->fAvailable = 0;
	result->wh.dwBufferLength = Globals->dwWaveBlockLen;
	result->wh.lpData = reinterpret_cast<LPSTR>(&result[1]);
	result->g = GlobalsActive;
	return result;
}

int WaveMix::MixerPlay(XWAVEHDR* lpXWH, int fWriteBlocks)
{
	if (!lpXWH)
		return 0;

	unsigned minStartPos = -1;
	auto playChannelCount = 0;
	auto channelPtr = Globals->aChannel;
	for (auto channelIndex = 16; channelIndex; --channelIndex)
	{
		auto channel = *channelPtr;
		if (channel != reinterpret_cast<CHANNELNODE*>(-1) && channel)
		{
			do
			{
				if (channel->dwEndPos > Globals->dwCurrentSample)
					break;
				channel = channel->next;
			}
			while (channel);
			if (channel)
			{
				if (channel->dwStartPos < minStartPos)
					minStartPos = channel->dwStartPos;
				play_channel_array[playChannelCount++] = channel;
			}
		}
		++channelPtr;
	}
	if (!playChannelCount)
	{
		if (fWriteBlocks)
			lpXWH->fAvailable = 1;
		return 0;
	}

	auto currentSample = Globals->dwCurrentSample;
	auto dataPtr = reinterpret_cast<unsigned char*>(lpXWH->wh.lpData);
	auto waveBlockLen = Globals->dwWaveBlockLen;
	while (waveBlockLen)
	{
		if (currentSample >= minStartPos)
		{
			auto waveCount = 0;
			auto endBlockPosition = currentSample + waveBlockLen;

			for (auto channelIndex = 0; channelIndex < playChannelCount; ++channelIndex)
			{
				auto channel = play_channel_array[channelIndex];
				if (channel->dwStartPos <= currentSample)
				{
					if (channel->dwEndPos < endBlockPosition)
						endBlockPosition = channel->dwEndPos;
					auto dataOffset = currentSample - channel->dwStartPos;
					if (channel->PlayParams.wLoops)
					{
						dataOffset %= channel->dwNumSamples;
						auto endBlockPosition2 = currentSample + (channel->dwNumSamples - dataOffset);
						if (endBlockPosition2 < endBlockPosition)
							endBlockPosition = endBlockPosition2;
					}
					play_data[waveCount] = &channel->lpPos[dataOffset];
					play_volume[waveCount].L = channel->Volume.L;
					play_volume[waveCount].R = channel->Volume.R;
					waveCount++;
				}
				else if (channel->dwStartPos < endBlockPosition)
				{
					endBlockPosition = channel->dwStartPos;
				}
			}

			if (waveCount)
			{
				auto dataLength = endBlockPosition - currentSample;
				Globals->CmixPtr(dataPtr, play_data, play_volume, waveCount, static_cast<WORD>(dataLength));

				dataPtr += dataLength;
				waveBlockLen -= dataLength;
				minStartPos = -1;
				currentSample += dataLength;


				auto playChPtr = play_channel_array;
				for (auto channelIndex = 0; channelIndex < playChannelCount;)
				{
					while (*playChPtr)
					{
						if ((*playChPtr)->dwEndPos > currentSample)
							break;
						*playChPtr = (*playChPtr)->next;
					}
					if (*playChPtr)
					{
						if ((*playChPtr)->dwStartPos < minStartPos)
							minStartPos = (*playChPtr)->dwStartPos;
						++channelIndex;
						++playChPtr;
					}
					else
					{
						playChannelCount--;
						*playChPtr = play_channel_array[playChannelCount];
						if (!playChannelCount)
							break;
					}
				}
			}
		}
		else
		{
			auto length = waveBlockLen;
			if (waveBlockLen + currentSample >= minStartPos)
				length = minStartPos - currentSample;
			memset(dataPtr, 0x80u, length);
			dataPtr += length;
			currentSample += length;
			waveBlockLen -= length;
		}
	}

	lpXWH->dwWavePos = Globals->dwCurrentSample;
	Globals->dwCurrentSample += Globals->dwWaveBlockLen;
	if (fWriteBlocks)
	{
		AddToPlayingQueue(lpXWH);
		if (waveOutWrite(Globals->hWaveOut, &lpXWH->wh, sizeof(WAVEHDR)))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Failed to write block to device", "WavMix32", MB_ICONWARNING);
			lpXWH->fAvailable = 1;
			RemoveFromPlayingQueue(lpXWH);
		}
	}
	return 1;
}

XWAVEHDR* WaveMix::AddToPlayingQueue(XWAVEHDR* lpXWH)
{
	lpXWH->QNext = nullptr;
	if (play_queue.first)
	{
		play_queue.last->QNext = lpXWH;
	}
	else
	{
		play_queue.first = lpXWH;
	}
	play_queue.last = lpXWH;
	return play_queue.first;
}

void WaveMix::MyWaveOutReset(HWAVEOUT hWaveOut)
{
	auto position = MyWaveOutGetPosition(hWaveOut, Globals->fGoodGetPos);
	waveOutReset(hWaveOut);
	SetWaveOutPosition(position);
}

void WaveMix::SetWaveOutPosition(unsigned newPosition)
{
	DWORD position;
	mmtime_tag pmmt{};

	pmmt.wType = TIME_BYTES;
	if (Globals->hWaveOut)
	{
		waveOutGetPosition(Globals->hWaveOut, &pmmt, 0xCu);
		position = pmmt.u.ms; /*Todo: check union ms vs sample with TIME_BYTES*/
	}
	else
	{
		position = 0;
	}

	if (position < newPosition)
	{
		Globals->dwWaveOutPos = newPosition - position;
		Globals->pfnSampleAdjust = AddFactor;
	}
	else
	{
		Globals->dwWaveOutPos = position - newPosition;
		Globals->pfnSampleAdjust = SubFactor;
	}
	Globals->dwCurrentSample = newPosition;
	Globals->dwBaseTime = timeGetTime() - 1000 * newPosition / Globals->PCM.wf.nAvgBytesPerSec;
}

DWORD WaveMix::SubFactor(DWORD a1, DWORD a2)
{
	return a1 - a2;
}

DWORD WaveMix::AddFactor(DWORD a1, DWORD a2)
{
	return a1 + a2;
}

dialog_template* WaveMix::MakeSettingsDlgTemplate()
{
	size_t size = 0u;
	dialog_template* temp = MakeDlgTemplate(&size, 0x80C80080, 0, 0, 212, 132, L"WavMix32 Settings (Ver %X.%X Static)");
	temp = AddDlgControl(&size, temp, 128, 65537, 1, 155, 5, 50, 14, L"&Ok");
	temp = AddDlgControl(&size, temp, 128, 0x10000, 2, 155, 25, 50, 14, L"&Cancel");
	temp = AddDlgControl(&size, temp, 130, 0, 1012, 5, 5, 115, 8, L"Number of WaveBlocks (%d-%d):");
	temp = AddDlgControl(&size, temp, 130, 0, 1013, 5, 20, 115, 8, L"Size of WaveBlocks (%d-%d):");
	temp = AddDlgControl(&size, temp, 130, 0, 0xFFFF, 5, 35, 102, 8, L"Playback Frequency (11,22,44):");
	temp = AddDlgControl(&size, temp, 130, 0, 0xFFFF, 5, 80, 57, 8, L"Playback Device:");
	temp = AddDlgControl(&size, temp, 130, 0, 1014, 5, 50, 115, 8, L"Number of Pause Blocks (%d-%d):");
	temp = AddDlgControl(&size, temp, 130, 0, 1015, 5, 65, 63, 8, L"Max Channels = %d");
	temp = AddDlgControl(&size, temp, 129, 8454272, 1003, 125, 5, 25, 12, L"-1");
	temp = AddDlgControl(&size, temp, 129, 8454272, 1007, 125, 20, 25, 12, L"-1");
	temp = AddDlgControl(&size, temp, 129, 8454272, 1008, 125, 35, 25, 12, L"-1");
	temp = AddDlgControl(&size, temp, 129, 8454272, 1011, 125, 50, 25, 12, L"-1");
	temp = AddDlgControl(&size, temp, 133, 2162946, 1009, 65, 80, 140, 60, L"No Devices");
	temp = AddDlgControl(&size, temp, 128, 65539, 1000, 5, 101, 40, 10, L"Stereo");
	temp = AddDlgControl(&size, temp, 128, 65539, 1001, 55, 101, 60, 10, L"Reset Remix");
	temp = AddDlgControl(&size, temp, 128, 65539, 1010, 125, 101, 78, 10, L"Show Debug Dialogs");
	temp = AddDlgControl(&size, temp, 128, 65539, 1005, 5, 117, 40, 10, L"CMixit");
	return AddDlgControl(&size, temp, 128, 65539, 1004, 55, 117, 75, 10, L"Good Get Position");
}

dialog_template* WaveMix::MakeDlgTemplate(size_t* totalSize, unsigned style, short x, short y, short cx, short cy,
                                          const wchar_t* String)
{
	auto dlgSize = 2 * wcslen(String) + 24;
	*totalSize = dlgSize;
	if ((dlgSize & 3) != 0)
		*totalSize = dlgSize - (dlgSize & 3) + 4;

	auto hGlobal = GlobalAlloc(GHND, *totalSize);
	auto dlgTemplate = static_cast<dialog_template*>(GlobalLock(hGlobal));
	if (dlgTemplate)
	{
		dlgTemplate->Dialog.dwExtendedStyle = 0;
		dlgTemplate->Dialog.cdit = 0;
		dlgTemplate->Dialog.style = style | 0x10000000;
		dlgTemplate->Dialog.x = x;
		dlgTemplate->Dialog.y = y;
		dlgTemplate->Dialog.cx = cx;
		dlgTemplate->Dialog.cy = cy;
		memcpy(dlgTemplate->Header, String, 2 * wcslen(String) + 2);
	}
	return dlgTemplate;
}

dialog_template* WaveMix::AddDlgControl(size_t* totalSize, dialog_template* dlgTemplate, short idClass,
                                        unsigned style, WORD id, short x, short y, short cx, short cy,
                                        const wchar_t* String)
{
	dialog_template* dlgTemplate2 = dlgTemplate;
	if (dlgTemplate)
	{
		auto prevSize = *totalSize;
		*totalSize += 2 * wcslen(String) + 25;
		if ((*totalSize & 3) != 0)
			*totalSize = *totalSize - (*totalSize & 3) + 4;

		GlobalUnlock(GlobalHandle(dlgTemplate2));
		auto newSize = *totalSize;
		HGLOBAL hGlobal = GlobalReAlloc(GlobalHandle(dlgTemplate2), newSize, 0x42u);
		dlgTemplate2 = static_cast<dialog_template*>(GlobalLock(hGlobal));
		if (dlgTemplate2)
		{
			auto dlgItem = (dialog_item_template*)((char*)dlgTemplate2 + prevSize);
			dlgItem->Item.dwExtendedStyle = 0;
			dlgItem->Item.style = style | 0x50000000;
			dlgItem->Item.x = x;
			dlgItem->Item.y = y;
			dlgItem->Item.cx = cx;
			dlgItem->Item.cy = cy;
			dlgItem->Item.id = id;
			dlgItem->sysClass = 0xFFFF;
			dlgItem->idClass = idClass;
			wcscpy_s(dlgItem->Header, wcslen(String) + 1, String);
			++dlgTemplate2->Dialog.cdit;
		}
	}
	return dlgTemplate2;
}

void WaveMix::DestroySettingsDlgTemplate(LPCVOID pMem)
{
	if (pMem)
	{
		GlobalUnlock(GlobalHandle(pMem));
		GlobalFree(GlobalHandle(pMem));
	}
}

int WaveMix::Settings_OnInitDialog(HWND hWnd, WPARAM wParam, MIXCONFIG* lpMixconfig)
{
	tagWAVEOUTCAPSA pwoc{};
	CHAR String[256];

	GetWindowTextA(hWnd, String, 256);
	wsprintfA(string_buffer, String, 2, 81);
	SetWindowTextA(hWnd, string_buffer);
	SetWindowLongPtr(hWnd, -21, reinterpret_cast<LONG_PTR>(lpMixconfig));
	SendMessageA(GetDlgItem(hWnd, 1000), 0xF1u, lpMixconfig->wChannels > 1u, 0);
	SendMessageA(GetDlgItem(hWnd, 1001), 0xF1u, lpMixconfig->ResetMixDefaultFlag != 0, 0);
	SendMessageA(GetDlgItem(hWnd, 1004), 0xF1u, lpMixconfig->GoodWavePos != 0, 0);
	SendMessageA(GetDlgItem(hWnd, 1005), 0xF1u, lpMixconfig->CmixPtrDefaultFlag != 0, 0);
	SendMessageA(GetDlgItem(hWnd, 1010), 0xF1u, lpMixconfig->ShowDebugDialogs != 0, 0);
	EnableWindow(GetDlgItem(hWnd, 1005), 0);
	SendMessageA(GetDlgItem(hWnd, 1003), 0xC5u, 2u, 0);
	SendMessageA(GetDlgItem(hWnd, 1007), 0xC5u, 4u, 0);
	SendMessageA(GetDlgItem(hWnd, 1008), 0xC5u, 2u, 0);
	SendMessageA(GetDlgItem(hWnd, 1011), 0xC5u, 2u, 0);
	GetWindowTextA(GetDlgItem(hWnd, 1012), String, 100);
	wsprintfA(string_buffer, String, 2, 10);
	SetWindowTextA(GetDlgItem(hWnd, 1012), string_buffer);
	GetWindowTextA(GetDlgItem(hWnd, 1013), String, 100);
	wsprintfA(string_buffer, String, 344, 11025);
	SetWindowTextA(GetDlgItem(hWnd, 1013), string_buffer);
	GetWindowTextA(GetDlgItem(hWnd, 1014), String, 100);
	wsprintfA(string_buffer, String, 0, 10);
	SetWindowTextA(GetDlgItem(hWnd, 1014), string_buffer);
	GetWindowTextA(GetDlgItem(hWnd, 1015), String, 100);
	wsprintfA(string_buffer, String, 16);
	SetWindowTextA(GetDlgItem(hWnd, 1015), string_buffer);
	wsprintfA(string_buffer, "%d", lpMixconfig->WaveBlockCount);
	SetWindowTextA(GetDlgItem(hWnd, 1003), string_buffer);
	wsprintfA(string_buffer, "%d", lpMixconfig->WaveBlockLen);
	SetWindowTextA(GetDlgItem(hWnd, 1007), string_buffer);
	wsprintfA(string_buffer, "%d", lpMixconfig->wSamplingRate);
	SetWindowTextA(GetDlgItem(hWnd, 1008), string_buffer);
	wsprintfA(string_buffer, "%d", lpMixconfig->PauseBlocks);
	SetWindowTextA(GetDlgItem(hWnd, 1011), string_buffer);

	signed int uDeviceID = 0;
	signed int deviceCount = waveOutGetNumDevs();
	if (deviceCount > 0)
	{
		do
		{
			MMRESULT getResult = waveOutGetDevCapsA(uDeviceID, &pwoc, 0x34u);
			if (getResult)
			{
				wsprintfA(string_buffer, "waveOutGetDevCaps failed (err %u) for device %d", getResult, uDeviceID);
				MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
			}
			else
			{
				wsprintfA(string_buffer, "%d: %s", uDeviceID, pwoc.szPname);
				SendMessageA(GetDlgItem(hWnd, 1009), 0x143u, 0, (LPARAM)string_buffer);
			}
			++uDeviceID;
		}
		while (uDeviceID < deviceCount);
	}
	SendMessageA(GetDlgItem(hWnd, 1009), 0x14Eu, lpMixconfig->wDeviceID, 0);
	return 1;
}

int WaveMix::Settings_OnCommand(HWND hWnd, int command, LPARAM lParam, int wParam)
{
	auto userData = reinterpret_cast<MIXCONFIG*>(GetWindowLongPtrA(hWnd, -21));
	if (command == 1)
	{
		if (userData)
		{
			userData->wChannels = (SendMessageA(GetDlgItem(hWnd, 1000), 0xF0u, 0, 0) != 0) + 1;
			userData->ResetMixDefaultFlag = SendMessageA(GetDlgItem(hWnd, 1001), 0xF0u, 0, 0) != 0;
			userData->GoodWavePos = SendMessageA(GetDlgItem(hWnd, 1004), 0xF0u, 0, 0) != 0;
			userData->ShowDebugDialogs = SendMessageA(GetDlgItem(hWnd, 1010), 0xF0u, 0, 0) != 0;
			userData->CmixPtrDefaultFlag = SendMessageA(GetDlgItem(hWnd, 1005), 0xF0u, 0, 0) != 0;
			GetWindowTextA(GetDlgItem(hWnd, 1003), string_buffer, 10);
			userData->WaveBlockCount = atoi(string_buffer);
			GetWindowTextA(GetDlgItem(hWnd, 1007), string_buffer, 10);
			userData->WaveBlockLen = atoi(string_buffer);
			GetWindowTextA(GetDlgItem(hWnd, 1008), string_buffer, 10);
			userData->wSamplingRate = atoi(string_buffer);
			GetWindowTextA(GetDlgItem(hWnd, 1011), string_buffer, 10);
			userData->PauseBlocks = atoi(string_buffer);
			GetWindowTextA(GetDlgItem(hWnd, 1009), string_buffer, 10);
			userData->wDeviceID = isdigit(string_buffer[0]) ? atoi(string_buffer) : 0;
			EndDialog(hWnd, 1);
		}
	}
	else
	{
		if (command != 2)
			return 0;
		EndDialog(hWnd, 0);
	}
	return 1;
}

int WaveMix::ReadRegistryToGetMachineSpecificInfSection(unsigned wDeviceId, LPSTR lpString1, int maxLength)
{
	HKEY phkResult;
	CHAR SubKey[52];

	int result = 0;
	int osPrefixLength = lstrlenA(GetOperatingSystemPrefix());
	if (maxLength < osPrefixLength + 10)
		return 0;
	wsprintfA(SubKey, "Device%u", wDeviceId);
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\WaveMix", &phkResult))
		return 0;
	lstrcpyA(lpString1, GetOperatingSystemPrefix());
	LONG cbData = maxLength - osPrefixLength;
	if (!RegQueryValueA(phkResult, SubKey, &lpString1[osPrefixLength], &cbData) && cbData > 0)
		result = 1;
	RegCloseKey(phkResult);
	return result;
}

#pragma warning (disable : 4996)/*Original uses GetVersion*/
const char* WaveMix::GetOperatingSystemPrefix()
{
	if (GetVersion() < 0x80000000)
		return "WinNT:";
	if (GetVersion() >= 0x80000000 && static_cast<uint8_t>(GetVersion()) >= 4u)
		return "Win95:";
	if (GetVersion() >= 0x80000000 && static_cast<uint8_t>(GetVersion()) < 4u)
		return "Win31:";
	return "OS_X"; /*The next big thing: waveOut on OSX*/
}
#pragma warning (default : 4996)/*Original uses GetVersion*/

unsigned WaveMix::FigureOutDMABufferSize(unsigned waveBlockLen, PCMWAVEFORMAT* pcm)
{
	unsigned int result = waveBlockLen;
	if (!waveBlockLen)
		result = pcm->wf.nSamplesPerSec * pcm->wf.nChannels * (pcm->wBitsPerSample >> 3) >> 4;
	if (result < 344)
		result = 344;
	if (result > 11025)
		result = 11025;
	return result;
}

int WaveMix::NoResetRemix(DWORD dwRemixSamplePos, CHANNELNODE* channel)
{
	Pump();
	return 1;
}

void WaveMix::SaveConfigSettings(unsigned dwFlags)
{
	if ((dwFlags & 0x80u) != 0)
	{
		wsprintfA(string_buffer, "%d", Globals->wDeviceID);
		WritePrivateProfileStringA("general", "WaveOutDevice", string_buffer, FileName);
	}
	if ((dwFlags & 1) != 0)
	{
		wsprintfA(string_buffer, "%d", Globals->PCM.wf.nChannels);
		WritePrivateProfileStringA(Globals->szDevicePName, "Channels", string_buffer, FileName);
	}
	if ((dwFlags & 2) != 0)
	{
		wsprintfA(string_buffer, "%d", static_cast<uint16_t>(11 * (Globals->PCM.wf.nSamplesPerSec / 0x2B11)));
		WritePrivateProfileStringA(Globals->szDevicePName, "SamplesPerSec", string_buffer, FileName);
	}
	if ((dwFlags & 4) != 0)
	{
		wsprintfA(string_buffer, "%d", Globals->WaveBlockCount);
		WritePrivateProfileStringA(Globals->szDevicePName, "WaveBlocks", string_buffer, FileName);
	}
	if ((dwFlags & 8) != 0)
	{
		wsprintfA(string_buffer, "%d", LOWORD(WaveMix::Globals->dwWaveBlockLen));
		WritePrivateProfileStringA(Globals->szDevicePName, "WaveBlockLen", string_buffer, FileName);
	}
	if ((dwFlags & 0x10) != 0)
	{
		wsprintfA(string_buffer, "%d", Globals->CmixPtr == cmixit);
		WritePrivateProfileStringA(Globals->szDevicePName, "CMixit", string_buffer, FileName);
	}
	if ((dwFlags & 0x20) != 0)
	{
		wsprintfA(string_buffer, "%d", (Globals->pfnRemix != ResetRemix) + 1);
		WritePrivateProfileStringA(Globals->szDevicePName, "Remix", string_buffer, FileName);
	}
	if ((dwFlags & 0x40) != 0)
	{
		wsprintfA(string_buffer, "%d", Globals->fGoodGetPos == 0);
		WritePrivateProfileStringA(Globals->szDevicePName, "GoodWavePos", string_buffer, FileName);
	}
	if ((dwFlags & 0x100) != 0)
	{
		wsprintfA(string_buffer, "%d", ShowDebugDialogs != 0);
		WritePrivateProfileStringA("general", "ShowDebugDialogs", string_buffer, FileName);
	}
	if ((dwFlags & 0x200) != 0)
	{
		wsprintfA(string_buffer, "%d", Globals->PauseBlocks);
		WritePrivateProfileStringA(Globals->szDevicePName, "PauseBlocks", string_buffer, FileName);
	}
}

void WaveMix::ShowCurrentSettings()
{
	tagWAVEOUTCAPSA pwoc{};

	if (waveOutGetDevCapsA(Globals->wDeviceID, &pwoc, 0x34u) || !RemoveInvalidIniNameCharacters(pwoc.szPname))
		lstrcpyA(pwoc.szPname, "Unknown Device");
	auto cmixitType = "cmixit";
	if (Globals->CmixPtr != cmixit)
		cmixitType = "386 mixit";
	auto getGood = "TRUE";
	if (!Globals->fGoodGetPos)
		getGood = "FALSE";
	auto remixType = "Reset";
	if (Globals->pfnRemix != ResetRemix)
		remixType = "NoReset";
	wsprintfA(
		string_buffer,
		"Using:\t%s.\n"
		"\tRemix = %s\n"
		"\tGoodGetPos=%s\n"
		"\t%d ping pong wave blocks\n"
		"\tWave block len = %lu bytes\n"
		"\tpfnMixit=%s\n"
		"\tSamplesPerSec=%lu,\n"
		"\tChannels=%d\n"
		"\tPauseBlocks=%d",
		pwoc.szPname,
		remixType,
		getGood,
		Globals->WaveBlockCount,
		Globals->dwWaveBlockLen,
		cmixitType,
		Globals->PCM.wf.nSamplesPerSec,
		Globals->PCM.wf.nChannels,
		Globals->PauseBlocks);
	MessageBoxA(nullptr, string_buffer, "WavMix32", MB_ICONINFORMATION);
}

unsigned WaveMix::GetWaveDevice()
{
	WAVEFORMATEX pwfx{};

	if (Globals->hWaveOut)
		return 0;
	HWND window = CreateWindowExA(0, "WavMix32", "", 0x8000000u, 0, 0, 0, 0, nullptr, nullptr, HModule,
	                              nullptr);
	GLOBALS* globals = Globals;
	Globals->hWndApp = window;
	if (!window)
	{
		if (ShowDebugDialogs)
			MessageBoxA(nullptr, "Failed to create callback window.", "WavMix32", MB_ICONWARNING);
		return 1;
	}
	pwfx.wFormatTag = globals->PCM.wf.wFormatTag;
	pwfx.nChannels = globals->PCM.wf.nChannels;
	pwfx.nSamplesPerSec = globals->PCM.wf.nSamplesPerSec;
	pwfx.nAvgBytesPerSec = globals->PCM.wf.nAvgBytesPerSec;
	pwfx.nBlockAlign = globals->PCM.wf.nBlockAlign;
	pwfx.wBitsPerSample = globals->PCM.wBitsPerSample;
	pwfx.cbSize = 0;
	unsigned int openResult = waveOutOpen(&globals->hWaveOut, 0xFFFFFFFF, &pwfx,
	                                      reinterpret_cast<DWORD_PTR>(globals->hWndApp), 0,
	                                      0x10000u);
	if (openResult)
	{
		DestroyWindow(Globals->hWndApp);
		Globals->hWndApp = nullptr;
		return openResult;
	}
	if (AllocWaveBlocks(Globals->hWaveOut, block_array1)
		&& AllocWaveBlocks(Globals->hWaveOut, block_array2))
	{
		Globals->WaveBlockArray = block_array1;
		return 0;
	}
	FreeWaveBlocks(Globals->hWaveOut, block_array1);
	FreeWaveBlocks(Globals->hWaveOut, block_array2);
	waveOutClose(Globals->hWaveOut);
	Globals->hWaveOut = nullptr;
	DestroyWindow(Globals->hWndApp);
	return 1;
}

void WaveMix::FreeWaveBlocks(HWAVEOUT hwo, XWAVEHDR** waveBlocks)
{
	for (int i = 0; i < 10; ++i)
	{
		auto blockPtr = &waveBlocks[i];
		if (*blockPtr)
		{
			waveOutUnprepareHeader(hwo, &(*blockPtr)->wh, sizeof(WAVEHDR));
			GlobalUnlock(GlobalHandle(*blockPtr));
			GlobalFree(GlobalHandle(*blockPtr));
			*blockPtr = nullptr;
		}
	}
}

int WaveMix::AllocWaveBlocks(HWAVEOUT hwo, XWAVEHDR** waveBlocks)
{
	for (int i = 0; i < 10; ++i)
	{
		auto xHDR = static_cast<XWAVEHDR*>(GlobalLock(GlobalAlloc(GMEM_SHARE, 0x2B41u)));
		waveBlocks[i] = xHDR;
		if (!xHDR)
		{
			if (ShowDebugDialogs)
				MessageBoxA(
					nullptr,
					"Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.",
					"WavMix32",
					MB_ICONWARNING);
			for (int j = i - 1; j >= 0; --j)
			{
				GlobalUnlock(GlobalHandle(waveBlocks[j]));
				GlobalFree(GlobalHandle(waveBlocks[j]));
				waveBlocks[j] = nullptr;
			}
			return 0;
		}
		xHDR->wh.lpData = (LPSTR)&xHDR[1];
		xHDR->wh.dwBufferLength = Globals->dwWaveBlockLen;
		xHDR->wh.dwFlags = 0;
		xHDR->wh.dwLoops = 0;
		xHDR->fAvailable = 1;
		xHDR->dwWavePos = 0;
	}
	int index = 0;
	while (!waveOutPrepareHeader(hwo, &waveBlocks[index]->wh, sizeof(WAVEHDR)))
	{
		waveBlocks[index++]->wh.dwFlags |= 1u;
		if (index >= 10)
			return 1;
	}
	if (ShowDebugDialogs)
		MessageBoxA(nullptr, "Unable to prepare wave header.", "WavMix32", MB_ICONWARNING);
	FreeWaveBlocks(hwo, waveBlocks);
	return 0;
}

void WaveMix::ReleaseWaveDevice(GLOBALS* globals)
{
	if (globals->fActive)
	{
		if (globals->hWaveOut)
		{
			MyWaveOutReset(globals->hWaveOut);
			DestroyPlayQueue();
			FreeWaveBlocks(globals->hWaveOut, block_array1);
			FreeWaveBlocks(globals->hWaveOut, block_array2);
			waveOutClose(globals->hWaveOut);
			globals->hWaveOut = nullptr;
			DestroyWindow(globals->hWndApp);
			globals->hWndApp = nullptr;
		}
	}
}

HPSTR WaveMix::WaveFormatConvert(PCMWAVEFORMAT* lpOutWF, PCMWAVEFORMAT* lpInWF, HPSTR lpInData, DWORD* dwDataSize)
{
	if (lpInWF->wf.nChannels == lpOutWF->wf.nChannels &&
		lpInWF->wf.nSamplesPerSec == lpOutWF->wf.nSamplesPerSec &&
		lpInWF->wBitsPerSample == lpOutWF->wBitsPerSample)
	{
		return lpInData;
	}
	HPSTR dataBuf = BitsPerSampleAlign(lpInData, lpInWF->wBitsPerSample, lpOutWF->wBitsPerSample, dwDataSize);
	if (!dataBuf)
		return nullptr;
	dataBuf = ChannelAlign(dataBuf, lpInWF->wf.nChannels, lpOutWF->wf.nChannels, lpOutWF->wBitsPerSample / 8,
	                       dwDataSize);
	if (!dataBuf)
		return nullptr;
	dataBuf = SamplesPerSecAlign(dataBuf, lpInWF->wf.nSamplesPerSec, lpOutWF->wf.nSamplesPerSec,
	                             lpOutWF->wBitsPerSample / 8, lpOutWF->wf.nChannels, dwDataSize);
	return dataBuf;
}

HPSTR WaveMix::BitsPerSampleAlign(HPSTR lpInData, WORD nInBPS, WORD nOutBPS, DWORD* dwDataSize)
{
	LPVOID dataBuf = nullptr;

	if (nInBPS == nOutBPS)
		return lpInData;

	if ((nInBPS == 8 || nInBPS == 16) && (nOutBPS == 8 || nOutBPS == 16))
	{
		DWORD dwNumSamples = *dwDataSize / (nInBPS / 8u);
		*dwDataSize = dwNumSamples * (nOutBPS / 8u);

		dataBuf = GlobalLock(GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, *dwDataSize));
		if (dataBuf)
		{
			if (nInBPS / 8u <= nOutBPS / 8u)
			{
				auto dst = static_cast<int16_t*>(dataBuf);
				for (auto src = lpInData; dwNumSamples; --dwNumSamples)
					*dst++ = static_cast<short>((*src++ - 128) * 256);
			}
			else
			{
				auto dst = static_cast<char*>(dataBuf);
				for (auto src = reinterpret_cast<int16_t*>(lpInData); dwNumSamples; --dwNumSamples)
				{
					*dst++ = static_cast<char>(*src++ / 256 + 128);
				}
			}
		}
		else
		{
			if (ShowDebugDialogs)
				MessageBoxA(
					nullptr,
					"Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.",
					"WavMix32",
					MB_ICONINFORMATION);
		}
	}

	GlobalUnlock(GlobalHandle(lpInData));
	GlobalFree(GlobalHandle(lpInData));
	return static_cast<HPSTR>(dataBuf);
}

HPSTR WaveMix::ChannelAlign(HPSTR lpInData, WORD nInChannels, WORD nOutChannels, WORD nBytesPerSample,
                            DWORD* dwDataSize)
{
	if (nInChannels == nOutChannels)
		return lpInData;
	DWORD dwNumSamples = *dwDataSize / nBytesPerSample / nInChannels;
	*dwDataSize = dwNumSamples * nBytesPerSample * nOutChannels;
	char* dataBuf = static_cast<char*>(GlobalLock(GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, *dwDataSize)));
	if (dataBuf)
	{
		if (nInChannels < nOutChannels)
		{
			if (nBytesPerSample == 1)
			{
				auto src = lpInData;
				auto dst = dataBuf;
				for (; dwNumSamples; --dwNumSamples)
				{
					*dst++ = *src;
					*dst++ = *src++;
				}
			}
			else
			{
				auto src = reinterpret_cast<short*>(lpInData);
				auto dst = reinterpret_cast<short*>(dataBuf);
				for (; dwNumSamples; --dwNumSamples)
				{
					*dst++ = *src;
					*dst++ = *src++;
				}
			}
		}
		else
		{
			if (nBytesPerSample == 1)
			{
				auto src = reinterpret_cast<unsigned char*>(lpInData);
				auto dst = reinterpret_cast<unsigned char*>(dataBuf);
				for (; dwNumSamples; --dwNumSamples, src += 2)
				{
					*dst++ = static_cast<unsigned char>((src[0] + src[1]) / 2);
				}
			}
			else
			{
				auto src = reinterpret_cast<int16_t*>(lpInData);
				auto dst = reinterpret_cast<int16_t*>(dataBuf);
				for (; dwNumSamples; --dwNumSamples, src += 2)
				{
					*dst++ = static_cast<short>((src[0] + src[1]) / 2);
				}
			}
		}
	}
	else
	{
		if (ShowDebugDialogs)
			MessageBoxA(
				nullptr,
				"Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.",
				"WavMix32",
				MB_ICONINFORMATION);
		dataBuf = nullptr;
	}

	GlobalUnlock(GlobalHandle(lpInData));
	GlobalFree(GlobalHandle(lpInData));
	return dataBuf;
}

HPSTR WaveMix::SamplesPerSecAlign(HPSTR lpInData, DWORD nInSamplesPerSec, DWORD nOutSamplesPerSec, WORD nBytesPerSample,
                                  WORD nChannels, DWORD* dwDataSize)
{
	if (nInSamplesPerSec == nOutSamplesPerSec)
		return lpInData;
	auto sampleSize = nBytesPerSample * nChannels;
	auto dwNumSamples = *dwDataSize / sampleSize;
	unsigned int nRep, nSkip, dwNumSamples2;
	if (nOutSamplesPerSec <= nInSamplesPerSec)
	{
		nRep = 0;
		nSkip = nInSamplesPerSec / nOutSamplesPerSec;
		dwNumSamples2 = dwNumSamples / nSkip;
	}
	else
	{
		nSkip = 0;
		nRep = nOutSamplesPerSec / nInSamplesPerSec;
		dwNumSamples2 = dwNumSamples * nRep;
	}
	*dwDataSize = sampleSize * dwNumSamples2;

	auto dataBuf = static_cast<char*>(GlobalLock(GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sampleSize * dwNumSamples2)));
	if (!dataBuf)
	{
		if (ShowDebugDialogs)
			MessageBoxA(
				nullptr,
				"Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.",
				"WavMix32",
				MB_ICONINFORMATION);
		GlobalUnlock(GlobalHandle(lpInData));
		GlobalFree(GlobalHandle(lpInData));
		return nullptr;
	}

	auto lpInDataBup = lpInData;
	auto dataBufBup = dataBuf;
	if (nRep <= 0)
	{
		for (auto index = dwNumSamples2 - 1; index; --index)
		{
			AvgSample(dataBuf, lpInData, nSkip, nBytesPerSample, nChannels);
			lpInData += sampleSize * nSkip;
			dataBuf += sampleSize;
		}
		for (; sampleSize; --sampleSize)
			*dataBuf++ = *lpInData++;
	}
	else
	{
		for (auto index = dwNumSamples - 1; index; --index)
		{
			RepSample(dataBuf, lpInData, nRep, nBytesPerSample, nChannels);
			lpInData += sampleSize;
			dataBuf += sampleSize * nRep;
		}
		for (auto index1 = nRep; index1; --index1)
		{
			auto src = lpInData;
			for (auto index2 = sampleSize; index2; --index2)
				*dataBuf++ = *src++;
		}
	}

	// Dump raw PCM for analysis.
	/*FILE* file;	
	fopen_s(&file,"wav1_dump.raw", "w");
	fwrite(dataBufBup, 1, sampleSize * dwNumSamples2, file);
	fclose(file);*/

	GlobalUnlock(GlobalHandle(lpInDataBup));
	GlobalFree(GlobalHandle(lpInDataBup));
	return dataBufBup;
}

void WaveMix::AvgSample(HPSTR lpOutData, HPSTR lpInData, unsigned nSkip, int nBytesPerSample, int nChannels)
{
	if (nBytesPerSample == 1)
	{
		auto dst = lpOutData;
		for (auto channelIndex = nChannels; channelIndex; --channelIndex)
		{
			auto src = lpInData++;
			auto average = 0;
			for (auto avgIndex = nSkip; avgIndex; --avgIndex)
			{
				average += static_cast<uint8_t>(*src) - 128;
				src += nChannels;
			}
			*dst++ = static_cast<char>(average / nSkip + 128);
		}
	}
	else
	{
		auto src = reinterpret_cast<int16_t*>(lpInData);
		auto dst = reinterpret_cast<int16_t*>(lpOutData);
		for (auto channelIndex = nChannels; channelIndex; --channelIndex)
		{
			auto curSrc = src++;
			auto average2 = 0;
			for (auto avgIndex = nSkip; avgIndex; --avgIndex)
			{
				average2 += *curSrc;
				curSrc += nChannels;
			}
			*dst++ = static_cast<short>(average2 / nSkip); /*Was *dst = */
		}
	}
}

void WaveMix::RepSample(HPSTR lpOutData, HPSTR lpInData, unsigned nRep, int nBytesPerSample, int nChannels)
{
	if (nBytesPerSample == 1)
	{
		auto src = reinterpret_cast<uint8_t*>(lpInData);
		auto dst = reinterpret_cast<uint8_t*>(lpOutData);
		for (auto channelIndex = nChannels; channelIndex; --channelIndex)
		{
			auto sample = *src;
			auto dst2 = &dst[nChannels];
			auto delta = (src[nChannels] - src[0]) / static_cast<int>(nRep);
			*dst = *src;
			dst++;
			for (auto repeatIndex = nRep - 1; repeatIndex; repeatIndex--)
			{
				sample += delta;
				*dst2 = sample;
				dst2 += nChannels;
			}
			++src;
		}
	}
	else
	{
		auto src = reinterpret_cast<int16_t*>(lpInData);
		auto dst = reinterpret_cast<int16_t*>(lpOutData);
		for (auto channelIndex2 = nChannels; channelIndex2; channelIndex2--)
		{
			auto sample = *src;
			auto dst2 = &dst[nChannels];
			auto delta = (src[nChannels] - src[0]) / static_cast<int>(nRep); /*Was dst[nChannels] - */
			*dst = *src;
			++dst;
			for (auto repeatIndex2 = nRep - 1; repeatIndex2; --repeatIndex2)
			{
				sample += delta;
				*dst2 = sample;
				dst2 += nChannels;
			}
			++src;
		}
	}
}

bool WaveMix::IsValidLPMIXWAVE(MIXWAVE* lpMixWave)
{
	return lpMixWave && lpMixWave->wMagic == 21554;
}

void WaveMix::FreePlayedBlocks()
{
	auto position = MyWaveOutGetPosition(Globals->hWaveOut, Globals->fGoodGetPos);
	for (int i = 0; i < MAXCHANNELS; i ++)
	{
		CHANNELNODE* channel = Globals->aChannel[i];
		if (channel && channel != reinterpret_cast<CHANNELNODE*>(-1))
		{
			while (channel && position >= channel->dwEndPos)
			{
				Globals->aChannel[i] = channel->next;
				if (channel->PlayParams.hWndNotify)
					PostMessageA(channel->PlayParams.hWndNotify, MM_WOM_DONE, i,
					             reinterpret_cast<LPARAM>(channel->lpMixWave));
				FreeChannelNode(channel);
				channel = Globals->aChannel[i];
			}
		}
	}
	if (!Globals->fGoodGetPos && !play_queue.first)
	{
		for (int i = 0; i < MAXCHANNELS; i++)
		{
			auto channel = Globals->aChannel[i];
			if (channel)
			{
				if (channel != reinterpret_cast<CHANNELNODE*>(-1))
				{
					PostMessageA(Globals->hWndApp, 0x400u, 0, reinterpret_cast<LPARAM>(Globals));
				}
			}
		}
	}
}

int WaveMix::HasCurrentOutputFormat(MIXWAVE* lpMixWave)
{
	return memcmp(&lpMixWave->pcm, &Globals->PCM, sizeof(PCMWAVEFORMAT)) == 0;
}

CHANNELNODE* WaveMix::GetChannelNode()
{
	CHANNELNODE* result = free_channel_nodes;
	if (result)
	{
		free_channel_nodes = free_channel_nodes->next;
		result->next = nullptr;
	}
	return result;
}

void WaveMix::ResetWavePosIfNoChannelData()
{
	if (!play_queue.first)
	{
		int channelIndex = 0;
		for (CHANNELNODE** i = Globals->aChannel; !*i || *i == reinterpret_cast<CHANNELNODE*>(-1); ++i)
		{
			if (++channelIndex >= 16)
			{
				SetWaveOutPosition(0);
				return;
			}
		}
	}
}

void WaveMix::cmixit(uint8_t* lpDest, uint8_t** rgWaveSrc, volume_struct* volumeArr, int iNumWaves,
                     uint16_t length)
{
	if (!length)
		return;

	if (Globals->PCM.wf.nChannels == 2)
	{
		if (iNumWaves == 1)
		{
			auto src = rgWaveSrc[0];
			for (auto index = (length - 1u) / 2u + 1u; index; --index)
			{
				*lpDest++ = volume_table[volumeArr->L][*src++];
				*lpDest++ = volume_table[volumeArr->R][*src++];
			}
		}
		else
		{
			for (auto srcOffset = 0u, index = (length - 1u) / 2u + 1u; index; index--)
			{
				auto sampleR = 128;
				auto sampleL = 128;
				auto volumePtr = volumeArr;
				for (auto channelIndex = 0; channelIndex < iNumWaves; channelIndex++)
				{
					auto src = rgWaveSrc[channelIndex] + srcOffset;
					sampleL += volume_table[volumePtr->L][src[0]] - 128;
					sampleR += volume_table[volumePtr->R][src[1]] - 128;
					++volumePtr;
				}

				srcOffset += 2;
				lpDest[0] = min(max(sampleL, 0), 255);
				lpDest[1] = min(max(sampleR, 0), 255);
				lpDest += 2;
			}
		}
	}
	else
	{
		if (iNumWaves == 1)
		{
			auto src = rgWaveSrc[0];
			auto avgVolume = (volumeArr->L + volumeArr->R) / 2;
			for (auto index = length; index; --index)
				*lpDest++ = volume_table[avgVolume][*src++];
		}
		else
		{
			for (unsigned srcOffset = 0u, index = length; index; index--)
			{
				auto sample = 128;
				auto volumePtr = volumeArr;
				for (auto channelIndex = 0; channelIndex < iNumWaves; channelIndex++)
				{
					auto src = rgWaveSrc[channelIndex] + srcOffset;
					auto curSample = volume_table[(volumePtr->L + volumePtr->R) / 2][src[0]];
					sample += curSample - 128;
					++volumePtr;
				}

				++srcOffset;
				*lpDest++ = min(max(sample, 0), 255);
			}
		}
	}
}

LRESULT WaveMix::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg != MM_WOM_DONE && Msg != WM_USER)
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	Pump();
	return 0;
}

INT_PTR WaveMix::SettingsDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG)
		return Settings_OnInitDialog(hWnd, wParam, reinterpret_cast<MIXCONFIG*>(lParam));
	if (Msg != WM_COMMAND)
		return 0;
	Settings_OnCommand(hWnd, LOWORD(wParam), lParam, HIWORD(wParam));
	return 1;
}
