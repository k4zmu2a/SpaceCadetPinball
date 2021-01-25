#include "pch.h"
#include "WaveMix.h"

int WaveMix::initialized_flag;
char WaveMix::FileName[276];
CHANNELNODE WaveMix::channel_nodes[MAXQUEUEDWAVES];
CHANNELNODE* WaveMix::free_channel_nodes;
char WaveMix::volume_table[256 * 11];
int WaveMix::debug_flag;
void (*WaveMix::cmixit_ptr)(unsigned __int8* lpDest, unsigned __int8** rgWaveSrc, volume_struct* volume, int iNumWaves,
                            unsigned __int16 length);
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

HANDLE WaveMix::Init()
{
	return ConfigureInit(nullptr);
}

HANDLE WaveMix::ConfigureInit(MIXCONFIG* lpConfig)
{
	MIXCONFIG mixConfig{};

	memset(&mixConfig, 0, 0x1Cu);
	unsigned int copySize = 30;
	mixConfig.RegistryKey = nullptr;
	mixConfig.wSize = 30;
	if (lpConfig)
	{
		if (lpConfig->wSize < 30u)
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
				MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
			}
			return nullptr;
		}

		if (GetPrivateProfileIntA("general", "ShowDevices", 0, FileName))
			ShowWaveOutDevices();
		auto globals = static_cast<GLOBALS*>(LocalAlloc(0x40u, 0x1C0u));
		Globals = globals;
		if (!globals)
			return nullptr;
		globals->CmixPtr = cmixit_ptr;
		globals->wMagic2 = 21554;
		globals->wMagic1 = 21554;
		globals->WaveBlockArray = nullptr;
		globals->SettingsDialogActiveFlag = 0;
		globals->unknown44 = 655370;
		memset(globals->aChannel, 0xFFu, sizeof globals->aChannel);
		memmove(&globals->PCM, &gpFormat, 0x10u);
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
	return 0;
}

int WaveMix::OpenChannel(HANDLE hMixSession, int iChannel, unsigned dwFlags)
{
	return 0;
}

int WaveMix::CloseChannel(HANDLE hMixSession, int iChannel, unsigned dwFlags)
{
	return 0;
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
	return new MIXWAVE{};
}

int WaveMix::FreeWave(HANDLE hMixSession, MIXWAVE* lpMixWave)
{
	return 0;
}

int WaveMix::Activate(HANDLE hMixSession, bool fActivate)
{
	return 0;
}

void WaveMix::Pump()
{
}

int WaveMix::Play(MIXPLAYPARAMS* lpMixPlayParams)
{
	return 0;
}

GLOBALS* WaveMix::SessionToGlobalDataPtr(HANDLE hMixSession)
{
	auto globals = static_cast<GLOBALS*>(hMixSession);
	if (hMixSession && globals->wMagic1 == 21554 && globals->wMagic2 == 21554)
		return globals;
	MessageBeep(0xFFFFFFFF);
	wsprintfA(string_buffer, "Invalid session handle 0x%04X passed to WaveMix API", hMixSession);
	MessageBoxA(nullptr, string_buffer, "WavMix32", 0x30u);
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
	int index2 = 0;
	int index3Sub = 0;
	char* tablePtr = &volume_table[128];
	do
	{
		int index1 = -128;
		int divSmth = index3Sub;
		do
		{
			tablePtr[index1] = static_cast<char>(divSmth / 10 + 128);
			divSmth += index2;
			++index1;
		}
		while (index1 < 128);
		++index2;
		index3Sub -= 128;
		tablePtr += 256;
	}
	while (tablePtr <= &volume_table[2688]);
}

void WaveMix::ShowWaveOutDevices()
{
	tagWAVEOUTCAPSA pwoc{};

	auto deviceCount = waveOutGetNumDevs();
	if (deviceCount)
	{
		wsprintfA(string_buffer, "%d waveOut Devices have been detected on your system.", deviceCount);
		MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
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
			MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
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
		lpConfig->ResetMixDefaultFlag = static_cast<unsigned __int16>(ReadRegistryInt(phkResult, "Remix", 1)) != 2;
	if ((dwFlags & 0x40) == 0)
	{
		int defaultGoodWavePos = DefaultGoodWavePos(lpConfig->wDeviceID);
		lpConfig->GoodWavePos = static_cast<unsigned __int16>(ReadRegistryInt(
			phkResult, "GoodWavePos", defaultGoodWavePos)) != 0;
	}
	if ((dwFlags & 0x100) == 0)
		lpConfig->ShowDebugDialogs = static_cast<short>(ReadRegistryInt(phkResult, "ShowDebugDialogs", 0));
	if ((dwFlags & 0x200) == 0)
	{
		int defaultPauseBlocks = DefaultPauseBlocks(static_cast<unsigned __int16>(lpConfig->WaveBlockCount));
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
	if (GetVersion() < 0x80000000 || static_cast<unsigned __int8>(GetVersion()) < 4u)
		result = waveBlocks;
	else
		result = 0;
	return result;
}
#pragma warning (default : 4996)

int WaveMix::Configure(GLOBALS* hMixSession, HWND hWndParent, MIXCONFIG* lpConfig, int* flag1Ptr, int saveConfigFlag)
{
	MIXCONFIG mixConfigLocal;

	auto hMixSession2 = hMixSession;
	auto mixConfig = lpConfig;
	auto hMixSession3 = hMixSession;
	auto flag1Ptr_2 = flag1Ptr;
	auto someFlag1 = 0;
	auto globals1 = SessionToGlobalDataPtr(hMixSession);
	Globals = globals1;
	if (!globals1)
		return 5;
	if (globals1->fActive)
		return 4;
	if (globals1->SettingsDialogActiveFlag)
		return 12;
	FlushChannel(hMixSession, -1, 1u);

	if (!mixConfig)
	{
		mixConfigLocal.wSize = 30;
		mixConfigLocal.dwFlags = 1023;
		GetConfig(static_cast<GLOBALS*>(hMixSession), &mixConfigLocal);
	}

	return 1;
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
		lpConfig->wSamplingRate = 11 * (globals->PCM.wf.nSamplesPerSec / 0x2B11);
	if ((dwFlags & 4) != 0)
		lpConfig->WaveBlockCount = globals->WaveBlockCount;
	if ((dwFlags & 8) != 0)
		lpConfig->WaveBlockLen = globals->WaveBlockLen;
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

unsigned WaveMix::MyWaveOutGetPosition(HWAVEOUT hwo, int fGoodGetPos)
{
	mmtime_tag pmmt{};

	if (!fGoodGetPos)
		return (timeGetTime() - Globals->dwBaseTime) * Globals->PCM.wf.nAvgBytesPerSec / 0x3E8 & 0xFFFFFFF8;
	pmmt.wType = 4;
	waveOutGetPosition(hwo, &pmmt, 0xCu);
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
	result->wh.dwBufferLength = Globals->WaveBlockLen;
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
	auto waveBlockLen = Globals->WaveBlockLen;
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
				Globals->CmixPtr(dataPtr, play_data, play_volume, waveCount, dataLength);

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
	Globals->dwCurrentSample += Globals->WaveBlockLen;
	if (fWriteBlocks)
	{
		AddToPlayingQueue(lpXWH);
		if (waveOutWrite(Globals->hWaveOut, &lpXWH->wh, 0x20u))
		{
			if (ShowDebugDialogs)
				MessageBoxA(nullptr, "Failed to write block to device", "WavMix32", 0x30u);
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

void WaveMix::cmixit(unsigned __int8* lpDest, unsigned __int8** rgWaveSrc, volume_struct* volume, int iNumWaves,
                     unsigned __int16 length)
{
}

LRESULT WaveMix::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg != 957 && Msg != 1024)
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	Pump();
	return 0;
}
