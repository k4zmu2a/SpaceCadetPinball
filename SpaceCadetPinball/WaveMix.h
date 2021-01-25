#pragma once

/* flag values for play params */
#define WMIX_QUEUEWAVE 0x00
#define WMIX_CLEARQUEUE  0x01
#define WMIX_USELRUCHANNEL 0x02
#define WMIX_HIPRIORITY 0x04
#define WMIX_WAIT 0x08

#define MAXCHANNELS 16
#define MAXQUEUEDWAVES 100

struct GLOBALS;

struct volume_struct
{
	unsigned __int16 L;
	unsigned __int16 R;
};


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
	int Unknown0;
};

struct CHANNELNODE
{
	CHANNELNODE* next;
	MIXPLAYPARAMS PlayParams;
	MIXWAVE* lpMixWave;
	DWORD dwNumSamples;
	DWORD dwStartPos;
	DWORD dwEndPos;
	unsigned char* lpPos;
	unsigned char* lpEnd;
	volume_struct Volume;
};

struct MIXCONFIG
{
	WORD wSize;
	DWORD dwFlags;
	WORD wChannels;
	WORD wSamplingRate;
	__int16 WaveBlockCount;
	__int16 WaveBlockLen;
	__int16 CmixPtrDefaultFlag;
	unsigned __int16 ResetMixDefaultFlag;
	unsigned __int16 GoodWavePos;
	unsigned __int16 wDeviceID;
	__int16 PauseBlocks;
	__int16 ShowDebugDialogs;
	HKEY RegistryKey;
};

struct XWAVEHDR
{
	WAVEHDR wh;
	BOOL fAvailable;
	DWORD dwWavePos;
	GLOBALS* g;
	struct XWAVEHDR* QNext;
};

struct PLAYQUEUE
{
	XWAVEHDR* first;
	XWAVEHDR* last;
};

struct GLOBALS
{
	WORD wMagic1;
	__int16 unknown0;
	int unknown1;
	int unknown2;
	HWAVEOUT hWaveOut;
	int fActive;
	int SettingsDialogActiveFlag;
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
	CHANNELNODE* aChannel[MAXCHANNELS];
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
	XWAVEHDR** WaveBlockArray;
	DWORD dwCurrentSample;
	DWORD dwBaseTime;
	int fGoodGetPos;
	int dwWaveOutPos;
	void (*CmixPtr)(unsigned __int8* lpDest, unsigned __int8** rgWaveSrc, volume_struct* volume, int iNumWaves,
	                unsigned __int16 length);
	int (* pfnRemix)(DWORD, CHANNELNODE*);
	int (* pfnSampleAdjust)(int, int);
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
	static int GetConfig(HANDLE hMixSession, MIXCONFIG* lpConfig);
	static unsigned MyWaveOutGetPosition(HWAVEOUT hwo, int fGoodGetPos);
	static void FreeChannelNode(CHANNELNODE* channel);
	static int ResetRemix(DWORD dwRemixSamplePos, CHANNELNODE* channel);
	static XWAVEHDR* RemoveFromPlayingQueue(XWAVEHDR* lpXWH);
	static void DestroyPlayQueue();
	static void SwapWaveBlocks();
	static XWAVEHDR* GetWaveBlock();
	static int MixerPlay(XWAVEHDR* lpXWH, int fWriteBlocks);
	static XWAVEHDR* AddToPlayingQueue(XWAVEHDR* lpXWH);
	static void cmixit(unsigned __int8* lpDest, unsigned __int8** rgWaveSrc, volume_struct* volume, int iNumWaves,
	                   unsigned __int16 length);
	static LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	static int initialized_flag;
	static char FileName[276];
	static CHANNELNODE channel_nodes[MAXQUEUEDWAVES];
	static CHANNELNODE* free_channel_nodes;
	static char volume_table[256 * 11];
	static int debug_flag;
	static void (*cmixit_ptr)(unsigned __int8* lpDest, unsigned __int8** rgWaveSrc, volume_struct* volume,
	                          int iNumWaves, unsigned __int16 length);
	static HMODULE HModule;
	static GLOBALS *Globals, *GlobalsActive;
	static PCMWAVEFORMAT gpFormat;
	static int ShowDebugDialogs;
	static char string_buffer[256];
	static PLAYQUEUE play_queue;
	static CHANNELNODE* play_channel_array[MAXCHANNELS];
	static XWAVEHDR* block_array1[10];
	static XWAVEHDR* block_array2[10];
	static unsigned char* play_data[MAXCHANNELS];
	static volume_struct play_volume[MAXCHANNELS];
};
