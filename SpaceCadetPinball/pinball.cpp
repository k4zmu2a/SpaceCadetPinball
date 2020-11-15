#include "pch.h"
#include "pinball.h"
#include "memory.h"


int pinball::quickFlag = 0;
TTextBox* pinball::InfoTextBox;
TTextBox* pinball::MissTextBox;
char pinball::getRcBuffer[6 * 256];
int pinball::rc_string_slot = 0;
HINSTANCE pinball::hinst;
char pinball::WindowName[2]{};
char pinball::DatFileName[300]{};
int pinball::LeftShift = -1;
int pinball::RightShift = -1;
HWND pinball::hwnd_frame = nullptr;
int pinball::has_focus = 1;
int pinball::single_step = 0;


char* pinball::get_rc_string(int uID, int a2)
{
	char* result = &getRcBuffer[256 * rc_string_slot];
	if (!LoadStringA(hinst, uID, &getRcBuffer[256 * rc_string_slot], 255))
		*result = 0;
	if (++rc_string_slot >= 6)
		rc_string_slot = 0;
	return result;
}

int pinball::get_rc_int(int uID, int* dst)
{
	char buffer[50]; 
	int result = LoadStringA(pinball::hinst, uID, buffer, 255);
	if (!result)
		return result;
	*dst = atoi(buffer);
	return 1;
}


void pinball::FindShiftKeys()
{
	signed int i; // esi
	int rightShift; // eax   
	CHAR stringBuf[10]; // [esp+Ch] [ebp-18h]

	RightShift = -1;
	LeftShift = -1;
	for (i = 0; i < 256; ++i)
	{
		if (MapVirtualKeyA(i, 1u) == 16)
		{
			LeftShift = i;
			break;
		}
	}
	while (++i < 256)
	{
		if (MapVirtualKeyA(i, 1u) == 16)
		{
			RightShift = i;
			break;
		}
	}
	if (!GetKeyNameTextA(LeftShift << 16, stringBuf, 19) || !_strnicmp(stringBuf, "right", 5u))
	{
		rightShift = RightShift;
	}
	else
	{
		rightShift = LeftShift;
		LeftShift = RightShift;
		RightShift = rightShift;
	}
	if (GetKeyNameTextA(rightShift << 16, stringBuf, 19))
	{
		if (_strnicmp(stringBuf, "left", 4u) != 0)
		{
			auto tmp = LeftShift;
			LeftShift = RightShift;
			RightShift = tmp;
		}
	}
}


HANDLE pinball::adjust_priority(int priority)
{
	HANDLE result = GetCurrentThread();
	switch (priority)
	{
	case -2:
		return (HANDLE)SetThreadPriority(result, -2);
	case -1:
		return (HANDLE)SetThreadPriority(result, -1);
	case 0:
		return (HANDLE)SetThreadPriority(result, 0);
	case 1:
		return (HANDLE)SetThreadPriority(result, 1);
	case 2:
		return (HANDLE)SetThreadPriority(result, 2);
	case 3:
		result = (HANDLE)SetThreadPriority(result, 15);
		break;
	}
	return result;
}
