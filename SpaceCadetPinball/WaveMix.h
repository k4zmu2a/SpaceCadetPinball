#pragma once

/* flag values for play params */
#define WMIX_QUEUEWAVE 0x00
#define WMIX_CLEARQUEUE  0x01
#define WMIX_USELRUCHANNEL 0x02
#define WMIX_HIPRIORITY 0x04
#define WMIX_WAIT 0x08

#define MAXCHANNELS 16

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

class WaveMix
{
public:
	static HANDLE Init();
	static int CloseSession(HANDLE hMixSession);
	static int OpenChannel(HANDLE hMixSession, int iChannel, unsigned int dwFlags);
	static int CloseChannel(HANDLE hMixSession, int iChannel, unsigned int dwFlags);
	static int FlushChannel(HANDLE hMixSession, int iChannel, unsigned int dwFlags);
	static MIXWAVE* OpenWave(HANDLE hMixSession, LPCSTR szWaveFilename, HINSTANCE hInst, unsigned int dwFlags);
	static int FreeWave(HANDLE hMixSession, MIXWAVE* lpMixWave);
	static int Activate(HANDLE hMixSession, bool fActivate);
	static void Pump();
	static int Play(MIXPLAYPARAMS* lpMixPlayParams);
};
