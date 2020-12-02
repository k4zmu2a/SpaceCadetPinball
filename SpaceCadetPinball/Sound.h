#pragma once
class Sound
{
public:
	static int Init(HINSTANCE hInstance, int voices, void (__stdcall* someFuncPtr)(int, int, int));
	static void Enable(int a1, int a2, int a3);
	static void nullsub_1(int a1, int a2, int a3);
	static void Idle();
	static void Activate();
	static void Deactivate();
	static void Close();
};
