#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pinball.h"
#include "resource.h"
#include "Sound.h"
#include "winmain.h"

LPCSTR options::OptionsRegPath;
LPSTR options::OptionsRegPathCur;
HMENU options::MenuHandle;
optionsStruct options::Options;

void options::init(HMENU menuHandle)
{
	MenuHandle = menuHandle;
	Options.Sounds = 1;
	Options.Music = 0;
	Options.FullScreen = 0;
	Options.Average = 5;
	Options.PriorityAdj = 2;
	Options.LeftFlipperKey2 = 90;
	Options.RightFlipperKey2 = 191;
	Options.PlungerKey2 = 32;
	Options.LeftTableBumpKey2 = 88;
	Options.RightTableBumpKey2 = 190;
	Options.BottomTableBumpKey2 = 38;
	pinball::get_rc_int(159, &Options.LeftFlipperKey2);
	pinball::get_rc_int(160, &Options.RightFlipperKey2);
	pinball::get_rc_int(161, &Options.PlungerKey2);
	pinball::get_rc_int(162, &Options.LeftTableBumpKey2);
	pinball::get_rc_int(163, &Options.RightTableBumpKey2);
	pinball::get_rc_int(164, &Options.BottomTableBumpKey2);
	Options.LeftFlipperKey = Options.LeftFlipperKey2;
	Options.RightFlipperKey = Options.RightFlipperKey2;
	Options.PlungerKey = Options.PlungerKey2;
	Options.LeftTableBumpKey = Options.LeftTableBumpKey2;
	Options.RightTableBumpKey = Options.RightTableBumpKey2;
	Options.Players = 1;
	Options.BottomTableBumpKey = Options.BottomTableBumpKey2;
	Options.Sounds = get_int(nullptr, "Sounds", Options.Sounds);
	Options.Music = get_int(nullptr, "Music", Options.Music);
	Options.Average = get_int(nullptr, "Average", Options.Average);
	Options.FullScreen = get_int(nullptr, "FullScreen", Options.FullScreen);
	Options.PriorityAdj = get_int(nullptr, "Priority_Adjustment", Options.PriorityAdj);
	Options.Players = get_int(nullptr, "Players", Options.Players);
	Options.LeftFlipperKey = get_int(nullptr, "Left Flippper key", Options.LeftFlipperKey);
	Options.RightFlipperKey = get_int(nullptr, "Right Flipper key", Options.RightFlipperKey);
	Options.PlungerKey = get_int(nullptr, "Plunger key", Options.PlungerKey);
	Options.LeftTableBumpKey = get_int(nullptr, "Left Table Bump key", Options.LeftTableBumpKey);
	Options.RightTableBumpKey = get_int(nullptr, "Right Table Bump key", Options.RightTableBumpKey);
	Options.BottomTableBumpKey = get_int(nullptr, "Bottom Table Bump key", Options.BottomTableBumpKey);
	menu_check(0xC9u, Options.Sounds);
	Sound::Enable(0, 7, Options.Sounds);
	menu_check(0xCAu, Options.Music);
	menu_check(0x193u, Options.FullScreen);
	menu_check(0x198u, Options.Players == 1);
	menu_check(0x199u, Options.Players == 2);
	menu_check(0x19Au, Options.Players == 3);
	menu_check(0x19Bu, Options.Players == 4);
	auto tmpBuf = memory::allocate(0x1F4u);
	if (tmpBuf)
	{
		get_string(nullptr, "Shell Exe", tmpBuf, pinball::WindowName, 500);
		if (!*tmpBuf)
		{
			if (MenuHandle)
			{
				DeleteMenu(MenuHandle, Menu1_Select_Table, 0);
				DrawMenuBar(winmain::hwnd_frame);
			}
		}
		memory::free(tmpBuf);
	}
}

void options::uninit()
{
	set_int(nullptr, "Sounds", Options.Sounds);
	set_int(nullptr, "Music", Options.Music);
	set_int(nullptr, "FullScreen", Options.FullScreen);
	set_int(nullptr, "Players", Options.Players);
	set_int(nullptr, "Left Flippper key", Options.LeftFlipperKey);
	set_int(nullptr, "Right Flipper key", Options.RightFlipperKey);
	set_int(nullptr, "Plunger key", Options.PlungerKey);
	set_int(nullptr, "Left Table Bump key", Options.LeftTableBumpKey);
	set_int(nullptr, "Right Table Bump key", Options.RightTableBumpKey);
	set_int(nullptr, "Bottom Table Bump key", Options.BottomTableBumpKey);
}

void options::path_init(LPCSTR regPath)
{
	char* buf = memory::allocate(lstrlenA(regPath) + 1);
	OptionsRegPath = buf;
	if (buf)
		lstrcpyA(buf, regPath);
}

void options::path_uninit()
{
	if (OptionsRegPath)
		memory::free((void*)OptionsRegPath);
	OptionsRegPath = nullptr;
}

LPCSTR options::path(LPCSTR regPath)
{
	char* buf = OptionsRegPathCur;
	if (!OptionsRegPathCur)
	{
		buf = memory::allocate(0x7D0u);
		OptionsRegPathCur = buf;
		if (!buf)
			return OptionsRegPath;
	}
	lstrcpyA(buf, OptionsRegPath);
	if (!regPath)
		return OptionsRegPathCur;
	lstrcatA(OptionsRegPathCur, "\\");
	lstrcatA(OptionsRegPathCur, regPath);
	return OptionsRegPathCur;
}

void options::path_free()
{
	if (OptionsRegPathCur)
		memory::free(OptionsRegPathCur);
	OptionsRegPathCur = nullptr;
}


int options::get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue)
{
	DWORD dwDisposition; // [esp+4h] [ebp-8h]	

	HKEY result = (HKEY)defaultValue, Data = (HKEY)defaultValue;
	if (!OptionsRegPath)
		return defaultValue;
	LPCSTR regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, &result, &dwDisposition))
	{
		optPath = (LPCSTR)4;
		RegQueryValueExA(result, lpValueName, nullptr, nullptr, (LPBYTE)&Data, (LPDWORD)&optPath);
		RegCloseKey(result);
	}
	path_free();
	return (int)Data;
}

void options::set_int(LPCSTR optPath, LPCSTR lpValueName, int data)
{
	DWORD dwDisposition; // [esp+4h] [ebp-4h]

	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&optPath,
		                     &dwDisposition))
		{
			RegSetValueExA((HKEY)optPath, lpValueName, 0, 4u, (const BYTE*)&data, 4u);
			RegCloseKey((HKEY)optPath);
		}
		path_free();
	}
}

void options::get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR lpString1, LPCSTR lpString2, int iMaxLength)
{
	const CHAR* v5 = (const CHAR*)iMaxLength;
	lstrcpynA(lpString1, lpString2, iMaxLength);
	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&iMaxLength,
		                     (LPDWORD)&optPath))
		{
			lpString2 = v5;
			RegQueryValueExA((HKEY)iMaxLength, lpValueName, nullptr, nullptr, (LPBYTE)lpString1, (LPDWORD)&lpString2);
			RegCloseKey((HKEY)iMaxLength);
		}
		path_free();
	}
}

void options::set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value)
{
	DWORD dwDisposition; // [esp+4h] [ebp-4h]

	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&optPath,
		                     &dwDisposition))
		{
			int v4 = lstrlenA(value);
			RegSetValueExA((HKEY)optPath, lpValueName, 0, 1u, (const BYTE*)value, v4 + 1);
			RegCloseKey((HKEY)optPath);
		}
		path_free();
	}
}


void options::menu_check(UINT uIDCheckItem, int check)
{
	if (MenuHandle)
		CheckMenuItem(MenuHandle, uIDCheckItem, check != 0 ? 8 : 0);
}

void options::menu_set(UINT uIDEnableItem, int enable)
{
	if (MenuHandle)
		EnableMenuItem(MenuHandle, uIDEnableItem, enable == 0);
}


void options::toggle(UINT uIDCheckItem)
{
	int newValue;
	switch (uIDCheckItem)
	{
	case Menu1_Sounds:
		newValue = Options.Sounds == 0;
		Options.Sounds = Options.Sounds == 0;
		Sound::Enable(0, 7, newValue);
		menu_check(uIDCheckItem, newValue);
		return;
	case Menu1_Music:
		newValue = Options.Music == 0;
		Options.Music = Options.Music == 0;
		if (!newValue)
			midi::music_stop();
		else
			midi::play_pb_theme(0);
		menu_check(uIDCheckItem, newValue);
		return;
	case Menu1_Full_Screen:
		newValue = Options.FullScreen == 0;
		Options.FullScreen = Options.FullScreen == 0;
		fullscrn::set_screen_mode(newValue);
		menu_check(uIDCheckItem, newValue);
		return;
	}
	if (uIDCheckItem > 407 && uIDCheckItem <= 411)
	{
		Options.Players = uIDCheckItem - 407;
		menu_check(0x198u, Options.Players == 1);
		menu_check(0x199u, Options.Players == 2);
		menu_check(0x19Au, Options.Players == 3);
		menu_check(0x19Bu, Options.Players == 4);
	}
}

void options::keyboard()
{
	DialogBoxParamA(winmain::hinst, "KEYMAPPER", winmain::hwnd_frame, KeyMapDlgProc, 0);
}

INT_PTR _stdcall options::KeyMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
