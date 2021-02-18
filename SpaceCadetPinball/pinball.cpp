#include "pch.h"
#include "pinball.h"
#include "memory.h"
#include "winmain.h"


int pinball::quickFlag = 0;
TTextBox* pinball::InfoTextBox;
TTextBox* pinball::MissTextBox;
char pinball::getRcBuffer[6 * 256];
int pinball::rc_string_slot = 0;
int pinball::LeftShift = -1;
int pinball::RightShift = -1;

char* pinball::get_rc_string(int uID, int a2)
{
	char* result = &getRcBuffer[256 * rc_string_slot];
	if (!LoadStringA(winmain::hinst, uID, &getRcBuffer[256 * rc_string_slot], 255))
		*result = 0;
	if (++rc_string_slot >= 6)
		rc_string_slot = 0;
	return result;
}

int pinball::get_rc_int(int uID, int* dst)
{
	char buffer[255];
	int result = LoadStringA(winmain::hinst, uID, buffer, 255);
	if (!result)
		return result;
	*dst = atoi(buffer);
	return 1;
}


void pinball::FindShiftKeys()
{
	signed int i;
	int rightShift;
	CHAR stringBuf[20];

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


void pinball::adjust_priority(int priority)
{
	auto thread = GetCurrentThread();
	switch (priority)
	{
	case -2:
		SetThreadPriority(thread, -2);
		break;
	case -1:
		SetThreadPriority(thread, -1);
		break;
	case 0:
		SetThreadPriority(thread, 0);
		break;
	case 1:
		SetThreadPriority(thread, 1);
		break;
	case 2:
		SetThreadPriority(thread, 2);
		break;
	case 3:
		SetThreadPriority(thread, 15);
		break;
	default:
		break;
	}
}

int pinball::make_path_name(LPSTR lpFilename, LPCSTR lpString2, int nSize)
{
	int nameSize = GetModuleFileNameA(nullptr, lpFilename, nSize);
	if (!nameSize || nameSize == nSize)
		return 1;
	for (CHAR* i = &lpFilename[nameSize]; i > lpFilename; --i)
	{
		if (*i == '\\' || *i == ':')
		{
			i[1] = 0;
			break;
		}
		--nameSize;
	}
	if (nameSize + 13 < nSize)
	{
		lstrcatA(lpFilename, lpString2);
		return 0;
	}
	lstrcatA(lpFilename, "?");
	return 1;
}
