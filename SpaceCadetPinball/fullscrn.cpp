#include "pch.h"
#include "fullscrn.h"


int fullscrn::screen_mode;
HWND fullscrn::hWnd;
tagRECT fullscrn::PubRect1;
int fullscrn::fullscrn_flag1;
int fullscrn::display_changed;
int fullscrn::ChangeDisplay, fullscrn::SmthFullScrnFlag2;
int fullscrn::trick;
int fullscrn::MenuEnabled;
HMENU fullscrn::MenuHandle;
int fullscrn::xDest, fullscrn::yDest;

int fullscrn::set_screen_mode(int isFullscreen)
{
	int result = isFullscreen;
	if (isFullscreen == screen_mode)
		return result;
	screen_mode = isFullscreen;
	if (isFullscreen)
	{
		if (IsWindowVisible(hWnd))
			GetWindowRect(hWnd, &PubRect1);
		enableFullscreen();
		BYTE1(fullscrn_flag1) |= 0x80u;
		InvalidateRect(hWnd, nullptr, 1);
		set_menu_mode(0);
		result = disableWindowFlagsDisDlg();
	}
	else
	{
		disableFullscreen();
		BYTE1(fullscrn_flag1) |= 0x80u;
		InvalidateRect(hWnd, nullptr, 1);
		set_menu_mode(1);
		result = RedrawWindow(nullptr, nullptr, nullptr, 0x185u);
	}
	return result;
}


int fullscrn::disableWindowFlagsDisDlg()
{
	long style = GetWindowLongA(hWnd, -16);
	return SetWindowLongA(hWnd, -16, style & 0xFF3FFFFF);
}

int fullscrn::setWindowFlagsDisDlg()
{
	int style = GetWindowLongA(hWnd, -16);
	return SetWindowLongA(hWnd, -16, style | 0xC00000);
}

int fullscrn::enableFullscreen()
{
	tagRECT Rect{}; // [esp+Ch] [ebp-B0h]
	DEVMODEA DevMode{}; // [esp+1Ch] [ebp-A0h]

	if (ChangeDisplay && !display_changed)
	{
		DevMode.dmSize = 156;
		DevMode.dmFields = 1835008;
		DevMode.dmPelsWidth = 640;
		DevMode.dmPelsHeight = 480;
		DevMode.dmBitsPerPel = 8;
		disableWindowFlagsDisDlg();
		if (trick)
		{
			GetWindowRect(GetDesktopWindow(), &Rect);
			SetWindowPos(hWnd, (HWND)((int)HWND_MESSAGE | 0x2), 0, 0, Rect.right - Rect.left + 1,
			             Rect.bottom - Rect.top + 1, 8u);
		}
		SmthFullScrnFlag2 = 1;
		LONG changeDispResult = ChangeDisplaySettingsA(&DevMode, 4u);
		if (changeDispResult == 1)
		{
			BYTE2(DevMode.dmFields) &= 0xFBu;
			SmthFullScrnFlag2 = 1;
			changeDispResult = ChangeDisplaySettingsA(&DevMode, 4u);
		}
		display_changed = changeDispResult == 0;
		if (changeDispResult == 0)
			return 1;
	}
	GetWindowRect(GetDesktopWindow(), &Rect);
	disableWindowFlagsDisDlg();
	SetWindowPos(hWnd, (HWND)0xFFFFFFFE, 0, 0, Rect.right - Rect.left + 1, Rect.bottom - Rect.top + 1, 8u);
	return 0;
}

int fullscrn::disableFullscreen()
{
	if (display_changed)
	{
		display_changed = 0;
		SmthFullScrnFlag2 = 1;
		ChangeDisplaySettingsA(nullptr, 4u);
		if (trick)
			SetWindowPos(hWnd, (HWND)0xFFFFFFFE, 0, 0, 0, 0, 0x13u);
	}
	setWindowFlagsDisDlg();
	SetWindowPos(
		hWnd,
		nullptr,
		PubRect1.left,
		PubRect1.top,
		PubRect1.right - PubRect1.left,
		PubRect1.bottom - PubRect1.top,
		0x14u);
	return 0;
}

bool fullscrn::set_menu_mode(int menuEnabled)
{
	BOOL result; // eax

	MenuEnabled = menuEnabled;
	GetWindowCenter();
	if (MenuEnabled)
	{
		fullscrn_flag1 |= 2u;
		InvalidateRect(hWnd, nullptr, 1);
		result = SetMenu(hWnd, MenuHandle);
	}
	else
	{
		fullscrn_flag1 |= 1u;
		InvalidateRect(hWnd, nullptr, 1);
		result = SetMenu(hWnd, nullptr);
	}
	return result;
}

void fullscrn::GetWindowCenter()
{
	int yDestLoc; // eax
	HWND v1; // eax
	struct tagRECT Rect; // [esp+4h] [ebp-10h]

	int dword_1025094 = 0, dword_1025098 = 0; // tmp, from render

	if (screen_mode)
	{
		v1 = GetDesktopWindow();
		GetWindowRect(v1, &Rect);
		xDest = (Rect.right - dword_1025094 - Rect.left) / 2;
		yDestLoc = (Rect.bottom - dword_1025098 - Rect.top) / 2;
	}
	else
	{
		xDest = 0;
		yDestLoc = GetSystemMetrics(15);
	}
	yDest = yDestLoc;
	if (MenuEnabled)
		yDest -= GetSystemMetrics(15);
}
