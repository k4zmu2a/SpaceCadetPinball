﻿#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pb.h"
#include "resource.h"
#include "Sound.h"
#include "winmain.h"

LPCSTR options::OptionsRegPath;
LPSTR options::OptionsRegPathCur;
HMENU options::MenuHandle;
optionsStruct options::Options{};

winhelp_entry options::keymap_help[18]
{
	winhelp_entry{0x1F5, 0x3EA},
	winhelp_entry{0x191, 0x3EC},
	winhelp_entry{0x192, 0x3ED},
	winhelp_entry{0x193, 0x3F1},
	winhelp_entry{0x194, 0x3EE},
	winhelp_entry{0x195, 0x3EF},
	winhelp_entry{0x196, 0x3F0},
	winhelp_entry{0x385, 0x3EC},
	winhelp_entry{0x386, 0x3ED},
	winhelp_entry{0x387, 0x3F1},
	winhelp_entry{0x388, 0x3EE},
	winhelp_entry{0x389, 0x3EF},
	winhelp_entry{0x38A, 0x3F0},
	winhelp_entry{0x38C, -1},
	winhelp_entry{0x38D, -1},
	winhelp_entry{0x321, -1},
	winhelp_entry{0x322, -1},
	winhelp_entry{0, 0},
};

short options::vk_list[28]
{
	-32703,
	0x5A,
	-32720,
	0x39,
	0x402E,
	0x402F,
	0x403B,
	0x4027,
	0x405B,
	0x405D,
	0x20,
	0x0D,
	0x9,
	0x14,
	0x25,
	0x27,
	0x26,
	0x28,
	0x2D,
	0x2E,
	0x24,
	0x23,
	0x21,
	0x22,
	0x90,
	0x91,
	0x13,
	-1
};

LanguageMenuEntry options::LanguageMenu[]
{
	{L"English", Languages::English},
	{L"Русский", Languages::Russian},
	{L"简化字", Languages::SimplifiedChinese},
	{L"正體字", Languages::TraditionalChinese},
	{L"Polski", Languages::Polish},
	{L"Deutsch", Languages::German},
	{L"日本", Languages::Japanese},
	{L"Français", Languages::French},
	{L"Italiano", Languages::Italian},
	{L"Magyar", Languages::Hungarian},
	{L"Português", Languages::Portuguese},
	{L"Português do Brasil", Languages::BrazilianPortuguese},
	{L"Čeština", Languages::Czech},
	{L"Dansk", Languages::Danish},
	{L"Suomi", Languages::Finnish},
	{L"עִברִית", Languages::Hebrew},
	{L"اَلْعَرَبِيَّة", Languages::Arabic},
	{L"Ελληνικά", Languages::Greek},
	{L"Español", Languages::Spanish},
	{L"한국어", Languages::Korean},
	{L"Nederlands", Languages::Dutch},
	{L"Norsk", Languages::Norwegian},
	{L"Svenska", Languages::Swedish},
	{L"Türkçe", Languages::Turkish},
};

void options::ReadOptions()
{
	Options.Sounds = 1;
	Options.Music = 0;
	Options.FullScreen = 0;
	Options.Average = 5;
	Options.PriorityAdj = 2;
	Options.LeftFlipperKeyDft = 90;
	Options.RightFlipperKeyDft = 191;
	Options.PlungerKeyDft = 32;
	Options.LeftTableBumpKeyDft = 88;
	Options.RightTableBumpKeyDft = 190;
	Options.BottomTableBumpKeyDft = 38;
	pinball::get_rc_int(159, &Options.LeftFlipperKeyDft);
	pinball::get_rc_int(160, &Options.RightFlipperKeyDft);
	pinball::get_rc_int(161, &Options.PlungerKeyDft);
	pinball::get_rc_int(162, &Options.LeftTableBumpKeyDft);
	pinball::get_rc_int(163, &Options.RightTableBumpKeyDft);
	pinball::get_rc_int(164, &Options.BottomTableBumpKeyDft);
	Options.LeftFlipperKey = Options.LeftFlipperKeyDft;
	Options.RightFlipperKey = Options.RightFlipperKeyDft;
	Options.PlungerKey = Options.PlungerKeyDft;
	Options.LeftTableBumpKey = Options.LeftTableBumpKeyDft;
	Options.RightTableBumpKey = Options.RightTableBumpKeyDft;
	Options.Players = 1;
	Options.BottomTableBumpKey = Options.BottomTableBumpKeyDft;
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
	Options.UniformScaling = get_int(nullptr, "Uniform scaling", true);
	Options.AlternativeRender = get_int(nullptr, "Alternative Render", false);
	Options.TargetUps = get_int(nullptr, "Target UPS", 120);
	Options.TargetUps = max(60, Options.TargetUps);
	Options.TargetUps = min(Options.TargetUps, 360);

	auto defaultLanguage = Languages::English;
	auto language = static_cast<Languages>(get_int(nullptr, "Language", static_cast<int>(defaultLanguage)));
	bool languageDefined = false;
	for (auto menuEntry : LanguageMenu)
	{
		if (menuEntry.Language == language)
		{
			languageDefined = true;
			break;
		}
	}
	if (!languageDefined)
		language = defaultLanguage;
	Options.Language = language;

	// Alternative approaches: resource DLLs, single-language builds.
	// SetThreadUILanguage does not work properly on Windows XP.	
	bool winXp = false;
	OSVERSIONINFO  version{};
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

#pragma warning (disable : 4996) // XP has no versionhelpers.h
	if (GetVersionEx(&version))
#pragma warning (default : 4996)
	{
		if (version.dwMajorVersion < 6)
			winXp = true;
	}

	if (winXp)
		SetThreadLocale(MAKELCID(static_cast<LANGID>(Options.Language), SORT_DEFAULT));
	else
		SetThreadUILanguage(static_cast<LANGID>(Options.Language));
}

void options::init(HMENU menuHandle)
{
	MenuHandle = menuHandle;

	menu_check(Menu1_Sounds, Options.Sounds);
	Sound::Enable(0, 7, Options.Sounds);
	menu_check(Menu1_Music, Options.Music);
	menu_check(Menu1_Full_Screen, Options.FullScreen);
	menu_check(Menu1_1Player, Options.Players == 1);
	menu_check(Menu1_2Players, Options.Players == 2);
	menu_check(Menu1_3Players, Options.Players == 3);
	menu_check(Menu1_4Players, Options.Players == 4);
	menu_check(Menu1_WindowUniformScale, Options.UniformScaling);
	menu_check(Menu1_AlternativeRender, Options.AlternativeRender);
	auto tmpBuf = memory::allocate(0x1F4u);
	if (tmpBuf)
	{
		get_string(nullptr, "Shell Exe", tmpBuf, "", 500);
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

	update_resolution_menu();

	// Add language menu from code to decouple it from rc.
	// AppendMenuW works with A window
	auto hSubmenu = CreatePopupMenu();
	auto index = Menu1_Language;
	for (auto menuEntry : LanguageMenu)
	{
		UINT flags = MF_STRING;
		if (menuEntry.Language == Options.Language)
			flags |= MF_CHECKED;
		AppendMenuW(hSubmenu, flags, index++, menuEntry.Name);
	}

	auto optionsMenu = GetSubMenu(MenuHandle, 1);	
	AppendMenu(optionsMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubmenu), "Language");
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
	set_int(nullptr, "Screen Resolution", Options.Resolution);
	set_int(nullptr, "Uniform scaling", Options.UniformScaling);
	set_int(nullptr, "Alternative Render", Options.AlternativeRender);
	set_int(nullptr, "Language", static_cast<int>(Options.Language));
	set_int(nullptr, "Target UPS", Options.TargetUps);
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
	DWORD dwDisposition;
	HKEY hKey;

	auto result = defaultValue;
	if (!OptionsRegPath)
		return result;

	auto regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition))
	{
		DWORD bufferSize = 4;
		RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&result), &bufferSize);
		RegCloseKey(hKey);
	}
	path_free();
	return result;
}

void options::set_int(LPCSTR optPath, LPCSTR lpValueName, int data)
{
	DWORD dwDisposition;
	HKEY hKey;

	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition))
	{
		RegSetValueExA(hKey, lpValueName, 0, 4u, reinterpret_cast<LPBYTE>(&data), 4u);
		RegCloseKey(hKey);
	}
	path_free();
}

void options::get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR dst, LPCSTR defaultValue, int iMaxLength)
{
	DWORD dwDisposition;
	HKEY hKey;

	lstrcpynA(dst, defaultValue, iMaxLength);
	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition))
	{
		DWORD bufferSize = iMaxLength;
		RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(dst), &bufferSize);
		RegCloseKey(hKey);
	}
	path_free();
}

void options::set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value)
{
	DWORD dwDisposition;
	HKEY hKey;

	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition))
	{
		RegSetValueExA(hKey, lpValueName, 0, 1u, LPBYTE(value), lstrlenA(value) + 1);
		RegCloseKey(hKey);
	}
	path_free();
}

void options::get_string(LPCSTR optPath, LPCWSTR lpValueName, LPWSTR dst, LPCWSTR defaultValue, int iMaxLength)
{
	DWORD dwDisposition;
	HKEY hKey;

	lstrcpynW(dst, defaultValue, iMaxLength);
	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition))
	{
		DWORD bufferSize = iMaxLength * sizeof(wchar_t);
		RegQueryValueExW(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(dst), &bufferSize);
		RegCloseKey(hKey);
	}
	path_free();
}

void options::set_string(LPCSTR optPath, LPCWSTR lpValueName, LPCWSTR value)
{
	DWORD dwDisposition;
	HKEY hKey;

	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition))
	{
		DWORD bufferSize = (lstrlenW(value) + 1) * sizeof(wchar_t);
		RegSetValueExW(hKey, lpValueName, 0, 1u, LPBYTE(value), bufferSize);
		RegCloseKey(hKey);
	}
	path_free();
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
	case Menu1_1Player:
	case Menu1_2Players:
	case Menu1_3Players:
	case Menu1_4Players:
		Options.Players = uIDCheckItem - Menu1_1Player + 1;
		menu_check(Menu1_1Player, Options.Players == 1);
		menu_check(Menu1_2Players, Options.Players == 2);
		menu_check(Menu1_3Players, Options.Players == 3);
		menu_check(Menu1_4Players, Options.Players == 4);
		break;
	case Menu1_MaximumResolution:
	case Menu1_640x480:
	case Menu1_800x600:
	case Menu1_1024x768:
		{
			for (unsigned i = Menu1_MaximumResolution; i <= Menu1_1024x768; ++i)
				menu_check(i, i == uIDCheckItem);

			int newResolution = uIDCheckItem - Menu1_640x480;
			if (uIDCheckItem == Menu1_MaximumResolution)
			{
				Options.Resolution = -1;
				if (fullscrn::GetMaxResolution() != fullscrn::GetResolution())
					winmain::Restart();
			}
			else if (newResolution != fullscrn::GetResolution() && newResolution <= fullscrn::GetMaxResolution())
			{
				Options.Resolution = newResolution;
				winmain::Restart();
			}
			break;
		}
	case Menu1_WindowUniformScale:
		Options.UniformScaling ^= true;
		menu_check(Menu1_WindowUniformScale, Options.UniformScaling);
		fullscrn::window_size_changed();
		fullscrn::paint();
		break;
	case Menu1_AlternativeRender:
		Options.AlternativeRender ^= true;
		menu_check(Menu1_AlternativeRender, Options.AlternativeRender);
		fullscrn::window_size_changed();
		fullscrn::paint();
		break;
	default:
		break;
	}

	if (uIDCheckItem >= Menu1_Language && uIDCheckItem < Menu1_LanguageMax)
	{
		auto languageId = uIDCheckItem - Menu1_Language;
		Options.Language = LanguageMenu[languageId].Language;
		winmain::Restart();
	}
}

void options::update_resolution_menu()
{
	auto maxResolution = fullscrn::get_max_supported_resolution();
	fullscrn::SetMaxResolution(maxResolution);
	PCWSTR maxResText = pinball::get_rc_Wstring(maxResolution + 2030, 0);
	if (MenuHandle)
		ModifyMenuW(MenuHandle, Menu1_MaximumResolution, 0, Menu1_MaximumResolution, maxResText);

	for (auto resIndex = 0; resIndex < 3; resIndex++)
	{
		menu_set(fullscrn::resolution_array[resIndex].ResolutionMenuId, fullscrn::GetMaxResolution() >= resIndex);
	}
	for (auto i = Menu1_MaximumResolution; i <= Menu1_1024x768; ++i)
	{
		menu_check(i, 0);
	}
	if (Options.Resolution >= 0)
		menu_check(fullscrn::resolution_array[fullscrn::GetResolution()].ResolutionMenuId, 1);
	else
		menu_check(Menu1_MaximumResolution, 1);
}

void options::init_resolution()
{
	Options.Resolution = get_int(nullptr, "Screen Resolution", -1);
	int maxRes = fullscrn::get_max_supported_resolution();
	if (Options.Resolution == -1 || maxRes <= Options.Resolution)
	{
		fullscrn::SetMaxResolution(maxRes);
		fullscrn::SetResolution(maxRes);
	}
	else
	{
		fullscrn::SetResolution(Options.Resolution);
	}
}

void options::keyboard()
{
	DialogBoxParamW(winmain::hinst, L"KEYMAPPER", winmain::hwnd_frame, KeyMapDlgProc, 0);
}

INT_PTR _stdcall options::KeyMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WCHAR keyName[20];
	int keyBindings[6];
	WCHAR rcString[256];

	switch (msg)
	{
	case WM_HELP:
		WinHelpA(static_cast<HWND>(reinterpret_cast<HELPINFO*>(lParam)->hItemHandle), "pinball.hlp", HELP_WM_HELP,
		         (ULONG_PTR)keymap_help);
		return 1;
	case WM_CONTEXTMENU:
		WinHelpA((HWND)wParam, "pinball.hlp", HELP_CONTEXTMENU, (ULONG_PTR)keymap_help);
		return 1;
	case WM_INITDIALOG:
		for (auto vkPtr = vk_list; *vkPtr != -1; vkPtr++)
		{
			short vk = *vkPtr;
			auto vk2And = vk & 0x4000;
			auto vkChar = static_cast<uint8_t>(vk);
			unsigned short maxVk;

			if (vk2And)
			{
				auto index = 128;
				do
				{
					if (vkChar == MapVirtualKeyW(index, MAPVK_VK_TO_CHAR))
						break;
					++index;
				}
				while (index < 256);

				if (index == 256)
				{
					continue;
				}

				keyName[0] = static_cast<char>(vkChar);
				keyName[1] = 0;
				vkChar = index;
				maxVk = index;
			}
			else
			{
				if (vk >= 0)
				{
					maxVk = vkChar;
				}
				else
				{
					++vkPtr;
					maxVk = *vkPtr;
				}
				if (vkChar > maxVk)
				{
					continue;
				}
			}

			for (int curVK = vkChar; curVK <= maxVk; curVK++)
			{
				if (vk2And || get_vk_key_name(curVK, keyName))
				{
					auto ind = SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL,CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.LeftFlipperKey)
						SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.RightFlipperKey)
						SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.PlungerKey)
						SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.LeftTableBumpKey)
						SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.RightTableBumpKey)
						SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.BottomTableBumpKey)
						SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_SETCURSEL, ind, 0);
				}
			}
		}
		return 1;
	case WM_COMMAND:
		switch (wParam)
		{
		case KEYMAPPER_Ok:
			{
				auto ind = SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL, CB_GETCURSEL, 0, 0);
				keyBindings[0] = static_cast<int>(SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL, CB_GETITEMDATA, ind, 0));
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_GETCURSEL, 0, 0);
				keyBindings[1] = static_cast<int>(SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_GETITEMDATA, ind, 0));
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_GETCURSEL, 0, 0);
				keyBindings[2] = static_cast<int>(SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_GETITEMDATA, ind, 0));
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_GETCURSEL, 0, 0);
				keyBindings[3] = static_cast<int>(SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_GETITEMDATA, ind, 0));
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_GETCURSEL, 0, 0);
				keyBindings[4] = static_cast<int>(SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_GETITEMDATA, ind, 0));
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_GETCURSEL, 0, 0);
				keyBindings[5] = static_cast<int>(SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_GETITEMDATA, ind, 0));

				auto sameKeyBound = 0;
				auto index = 1;
				auto optPtr = keyBindings;
				while (!sameKeyBound)
				{
					for (auto keyInd = index; keyInd < 6; keyInd++)
					{
						if (sameKeyBound)
							break;
						if (*optPtr == keyBindings[keyInd])
						{
							lstrcpyW(rcString, pinball::get_rc_Wstring(43, 0));
							MessageBoxW(hDlg, pinball::get_rc_Wstring(39, 0), rcString, 0x2000u);
							sameKeyBound = 1;
						}
					}
					++index;
					++optPtr;
					if (index - 1 >= 5)
					{
						if (sameKeyBound)
							return 1;
						Options.LeftFlipperKey = keyBindings[0];
						Options.RightFlipperKey = keyBindings[1];
						Options.PlungerKey = keyBindings[2];
						Options.LeftTableBumpKey = keyBindings[3];
						Options.RightTableBumpKey = keyBindings[4];
						Options.BottomTableBumpKey = keyBindings[5];
						EndDialog(hDlg, wParam);
						return 1;
					}
				}
				return 1;
			}
		case KEYMAPPER_Cancel:
			EndDialog(hDlg, wParam);
			return 1;
		case KEYMAPPER_Default:
			{
				auto name = (LPARAM)get_vk_key_name(Options.LeftFlipperKeyDft, keyName);
				auto ind = SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperL, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.RightFlipperKeyDft, keyName);
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageW(hDlg, KEYMAPPER_FlipperR, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.PlungerKeyDft, keyName);
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageW(hDlg, KEYMAPPER_Plunger, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.LeftTableBumpKeyDft, keyName);
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageW(hDlg, KEYMAPPER_BumpLeft, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.RightTableBumpKeyDft, keyName);
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageW(hDlg, KEYMAPPER_BumpRight, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.BottomTableBumpKeyDft, keyName);
				ind = SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageW(hDlg, KEYMAPPER_BumpBottom, CB_SETCURSEL, ind, 0);
				return 0;
			}
		default:
			return 0;
		}
	default:
		break;
	}

	return 0;
}


LPWSTR options::get_vk_key_name(uint16_t vk, LPWSTR keyName)
{
	LONG scanCode = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC) << 16;
	if (vk >= 0x21u && vk <= 0x2Eu)
		scanCode |= 0x1000000u;
	return GetKeyNameTextW(scanCode, keyName, 19) != 0 ? keyName : nullptr;
}
