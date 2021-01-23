#include "pch.h"
#include "WaveMix.h"

HANDLE WaveMix::Init()
{
	return (HANDLE)1;
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
