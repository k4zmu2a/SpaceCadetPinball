#include "pch.h"
#include "WaveMix.h"

int WaveMix::initialized_flag;
char WaveMix::FileName[276];
CHANNELNODE WaveMix::channel_nodes[MAXQUEUEDWAVES];
CHANNELNODE* WaveMix::free_channel_nodes;
char WaveMix::volume_table[256 * 11];
int WaveMix::debug_flag;
int WaveMix::cmixit_ptr;
HMODULE WaveMix::HModule;
PCMWAVEFORMAT WaveMix::gpFormat = {{1u, 1u, 11025u, 11025u, 1u}, 8u};
char WaveMix::string_buffer[256] = "WaveMix V 2.3 by Angel M. Diaz, Jr. (c) Microsoft 1993-1995";
GLOBALS* WaveMix::Globals;
int WaveMix::ShowDebugDialogs;

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
		globals->unknown102 = 0;
		globals->unknown5 = 0;
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
	cmixit_ptr = (int)cmixit;
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
		lpConfig->WaveBlocks = static_cast<short>(ReadRegistryInt(phkResult, "WaveBlocks", 3));
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
		int defaultPauseBlocks = DefaultPauseBlocks(static_cast<unsigned __int16>(lpConfig->WaveBlocks));
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
		result = (LOBYTE(pwoc.dwSupport) >> 5) & 1;
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
	return 1;
}

unsigned WaveMix::MyWaveOutGetPosition(HWAVEOUT hwo, int fGoodGetPos)
{
	mmtime_tag pmmt{};

	if (!fGoodGetPos)
		return ((timeGetTime() - Globals->dwBaseTime) * Globals->PCM.wf.nAvgBytesPerSec / 0x3E8) & 0xFFFFFFF8;
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

short WaveMix::cmixit(BYTE* a1, char* a2, char* a3, int a4, unsigned short a5)
{
	return 0;
}

LRESULT WaveMix::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg != 957 && Msg != 1024)
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	Pump();
	return 0;
}
