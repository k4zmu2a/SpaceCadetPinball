#include "pch.h"
#include "WaveMix.h"

#include "pinball.h"

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
		MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
		return 0;
	}
	if (GetPrivateProfileIntA("not compatible", Globals->szDevicePName, 0, FileName))
	{
		if (!ShowDebugDialogs)
			return 0;
		wsprintfA(string_buffer, "%s is not compatible with the realtime wavemixer.", Globals->szDevicePName);
		MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
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
		mixConfigLocal.wSize = 30;
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
	mmtime_tag pmmt{};

	if (!fGoodGetPos)
		return (timeGetTime() - Globals->dwBaseTime) * Globals->PCM.wf.nAvgBytesPerSec / 0x3E8 & 0xFFFFFFF8;
	pmmt.wType = TIME_BYTES;
	waveOutGetPosition(hWaveOut, &pmmt, 0xCu);
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
				MessageBoxA(nullptr, "Failed to write block to device", "WavMix32", 0x30u);
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
	auto size = 0u;
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

dialog_template* WaveMix::MakeDlgTemplate(unsigned* totalSize, unsigned style, short x, short y, short cx, short cy,
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

dialog_template* WaveMix::AddDlgControl(unsigned* totalSize, dialog_template* dlgTemplate, short idClass,
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
		unsigned int newSize = *totalSize;
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

int WaveMix::Settings_OnInitDialog(HWND hWnd, int wParam, MIXCONFIG* lpMixconfig)
{
	tagWAVEOUTCAPSA pwoc{};
	CHAR String[256];

	GetWindowTextA(hWnd, String, 256);
	wsprintfA(string_buffer, String, 2, 81);
	SetWindowTextA(hWnd, string_buffer);
	SetWindowLongA(hWnd, -21, (LONG)lpMixconfig);
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
				MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
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

int WaveMix::Settings_OnCommand(HWND hWnd, int command, int lParam, int wParam)
{
	auto userData = (MIXCONFIG*)GetWindowLongA(hWnd, -21);
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
	if (GetVersion() >= 0x80000000 && static_cast<unsigned __int8>(GetVersion()) >= 4u)
		return "Win95:";
	if (GetVersion() >= 0x80000000 && static_cast<unsigned __int8>(GetVersion()) < 4u)
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
		wsprintfA(string_buffer, "%d", static_cast<unsigned __int16>(11 * (Globals->PCM.wf.nSamplesPerSec / 0x2B11)));
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
	MessageBoxA(nullptr, string_buffer, "WavMix32", 0x40u);
}

unsigned WaveMix::GetWaveDevice()
{
	WAVEFORMATEX pwfx{};

	if (Globals->hWaveOut)
		return 0;
	HWND window = CreateWindowExA(0, "WavMix32", pinball::WindowName, 0x8000000u, 0, 0, 0, 0, nullptr, nullptr, HModule,
	                              nullptr);
	GLOBALS* globals = Globals;
	Globals->hWndApp = window;
	if (!window)
	{
		if (ShowDebugDialogs)
			MessageBoxA(nullptr, "Failed to create callback window.", "WavMix32", 0x30u);
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
					0x30u);
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
		MessageBoxA(nullptr, "Unable to prepare wave header.", "WavMix32", 0x30u);
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

INT_PTR WaveMix::SettingsDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == 272)
		return Settings_OnInitDialog(hWnd, wParam, (MIXCONFIG*)lParam);
	if (Msg != 273)
		return 0;
	Settings_OnCommand(hWnd, static_cast<unsigned __int16>(wParam), lParam, HIWORD(wParam));
	return 1;
}
