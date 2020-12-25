#include "pch.h"
#include "Sound.h"

int Sound::Init(HINSTANCE hInstance, int voices, void (__stdcall* someFuncPtr)(int, int, int))
{
	return 1;
}

void Sound::Enable(int a1, int a2, int a3)
{
}

void Sound::nullsub_1(int a1, int a2, int a3)
{
}

void Sound::Idle()
{
}

void Sound::Activate()
{
}

void Sound::Deactivate()
{
}

void Sound::Close()
{
}

int Sound::SubFactor(int a1, int a2)
{
	return a1 - a2;
}

int Sound::AddFactor(int a1, int a2)
{
	return a1 + a2;
}

void Sound::PlaySoundA(int a1, int a2, int a3, unsigned short a4, short a5)
{
}

CHAR* Sound::LoadWaveFile(LPCSTR lpName)
{
	return nullptr;
}

LPCVOID Sound::FreeSound(LPCVOID pMem)
{
	return nullptr;
}
