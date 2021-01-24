#pragma once

/* flag values for play params */
#define WMIX_QUEUEWAVE 0x00
#define WMIX_CLEARQUEUE  0x01
#define WMIX_USELRUCHANNEL 0x02
#define WMIX_HIPRIORITY 0x04
#define WMIX_WAIT 0x08

#define MAXCHANNELS 16
#define MAXQUEUEDWAVES 100

struct MIXWAVE
{
	PCMWAVEFORMAT pcm;
	WAVEHDR wh;
	char szWaveFilename[16];
	short Unknown0;
};

struct MIXPLAYPARAMS
{
	WORD wSize;
	HANDLE hMixSession;
	int iChannel;
	MIXWAVE* lpMixWave;
	HWND hWndNotify;
	DWORD dwFlags;
	WORD wLoops;
};

struct CHANNELNODE
{
	CHANNELNODE* next;
	MIXPLAYPARAMS PlayParams;
	MIXWAVE* lpMixWave;
	DWORD dwNumSamples;
	DWORD dwStartPos;
	DWORD dwEndPos;
	char* lpPos;
	char* lpEnd;
	int Unknown0;
	int Unknown1;
};

struct MIXCONFIG
{
	WORD wSize;
	DWORD dwFlags;
	WORD wChannels;
	WORD wSamplingRate;
	__int16 WaveBlocks;
	__int16 WaveBlockLen;
	__int16 CmixPtrDefaultFlag;
	unsigned __int16 ResetMixDefaultFlag;
	unsigned __int16 GoodWavePos;
	unsigned __int16 wDeviceID;
	__int16 PauseBlocks;
	__int16 ShowDebugDialogs;
	HKEY RegistryKey;
};

struct GLOBALS
{
	WORD wMagic1;
	__int16 unknown0;
	int unknown1;
	int unknown2;
	HWAVEOUT hWaveOut;
	int fActive;
	int unknown5;
	unsigned int wDeviceID;
	int unknown7;
	int unknown8;
	int unknown9;
	int unknown10;
	int unknown11;
	int unknown12;
	int unknown13;
	int unknown14;
	int unknown15;
	int unknown16;
	int unknown17;
	int unknown18;
	int unknown19;
	int unknown20;
	int unknown21;
	int unknown22;
	int unknown23;
	int unknown24;
	int unknown25;
	int unknown26;
	int unknown27;
	int unknown28;
	int unknown29;
	int unknown30;
	WAVEOUTCAPSA WaveoutCaps;
	int unknown44;
	int unknown45;
	int unknown46;
	int unknown47;
	int unknown48;
	int unknown49;
	int unknown50;
	int unknown51;
	int unknown52;
	int unknown53;
	int unknown54;
	int unknown55;
	int unknown56;
	int unknown57;
	int unknown58;
	int unknown59;
	int unknown60;
	CHANNELNODE* aChannel[16];
	int unknown77;
	int unknown78;
	int unknown79;
	int unknown80;
	int unknown81;
	int unknown82;
	int unknown83;
	int unknown84;
	int unknown85;
	int unknown86;
	int unknown87;
	int unknown88;
	int unknown89;
	int unknown90;
	int unknown91;
	int unknown92;
	int unknown93;
	int unknown94;
	PCMWAVEFORMAT PCM;
	int WaveBlockLen;
	int WaveBlockCount;
	int PauseBlocks;
	int unknown102;
	int unknown103;
	DWORD dwBaseTime;
	int fGoodGetPos;
	int dwWaveOutPos;
	int CmixPtr;
	void(__stdcall* pfnRemix)(DWORD, CHANNELNODE*);
	int (__stdcall* pfnSampleAdjust)(int, int);
	int unknown110;
	__int16 wMagic2;
	__int16 unknown112;
};


class WaveMix
{
public:
	static HANDLE Init();
	static HANDLE ConfigureInit(MIXCONFIG* lpConfig);
	static int CloseSession(HANDLE hMixSession);
	static int OpenChannel(HANDLE hMixSession, int iChannel, unsigned int dwFlags);
	static int CloseChannel(HANDLE hMixSession, int iChannel, unsigned int dwFlags);
	static int FlushChannel(HANDLE hMixSession, int iChannel, unsigned int dwFlags);
	static MIXWAVE* OpenWave(HANDLE hMixSession, LPCSTR szWaveFilename, HINSTANCE hInst, unsigned int dwFlags);
	static int FreeWave(HANDLE hMixSession, MIXWAVE* lpMixWave);
	static int Activate(HANDLE hMixSession, bool fActivate);
	static void Pump();
	static int Play(MIXPLAYPARAMS* lpMixPlayParams);

private:
	static GLOBALS* SessionToGlobalDataPtr(HANDLE hMixSession);
	static int Startup(HMODULE hModule);
	static int SetIniFileName(HMODULE hModule);
	static void InitChannelNodes();
	static void InitVolumeTable();
	static void ShowWaveOutDevices();
	static int RemoveInvalidIniNameCharacters(char* lpString);
	static int ReadConfigSettings(MIXCONFIG* lpConfig);
	static int ReadRegistryForAppSpecificConfigs(MIXCONFIG* lpConfig);
	static int ReadRegistryInt(HKEY hKey, LPCSTR lpSubKey, int defaultValue);
	static int DefaultGoodWavePos(unsigned int uDeviceID);
	static int DefaultPauseBlocks(int waveBlocks);
	static int Configure(GLOBALS* hMixSession, HWND hWndParent, MIXCONFIG* lpConfig, int* flag1Ptr, int saveConfigFlag);
	static unsigned MyWaveOutGetPosition(HWAVEOUT hwo, int fGoodGetPos);
	static void FreeChannelNode(CHANNELNODE* channel);
	static __int16 cmixit(BYTE* a1, char* a2, char* a3, int a4, unsigned __int16 a5);
	static LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	static int initialized_flag;
	static char FileName[276];
	static CHANNELNODE channel_nodes[MAXQUEUEDWAVES];
	static CHANNELNODE* free_channel_nodes;
	static char volume_table[256 * 11];
	static int debug_flag;
	static int cmixit_ptr;
	static HMODULE HModule;
	static GLOBALS* Globals;
	static PCMWAVEFORMAT gpFormat;
	static int ShowDebugDialogs;
	static char string_buffer[256];
};
