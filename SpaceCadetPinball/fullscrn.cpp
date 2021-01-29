#include "pch.h"
#include "fullscrn.h"
#include "render.h"
#include "winmain.h"


int fullscrn::screen_mode;
HWND fullscrn::hWnd;
tagRECT fullscrn::WindowRect1, fullscrn::WindowRect2;
rectangle_type fullscrn::WHRect;
int fullscrn::fullscrn_flag1;
int fullscrn::display_changed;
int fullscrn::ChangeDisplay, fullscrn::SmthFullScrnFlag2;
int fullscrn::trick = 1;
int fullscrn::MenuEnabled;
HMENU fullscrn::MenuHandle;

void fullscrn::init(int width, int height, int isFullscreen, HWND winHandle, HMENU menuHandle, int changeDisplay)
{
	WHRect.XPosition = 0;
	WHRect.YPosition = 0;
	ChangeDisplay = changeDisplay;
	hWnd = winHandle;
	MenuHandle = menuHandle;
	WHRect.Width = width;
	WHRect.Height = height;

	GetWindowRect(GetDesktopWindow(), &fullscrn::WindowRect1);
	int widht2 = width + 2 * GetSystemMetrics(SM_CXBORDER);
	int height2 = height + 2 * GetSystemMetrics(SM_CYBORDER);
	int menuHeight = GetSystemMetrics(SM_CYMENU);
	int captionHeight = GetSystemMetrics(SM_CYCAPTION);
	int borderHeight = WindowRect1.bottom - WindowRect1.top - height2;

	WindowRect2.bottom = borderHeight / 2 - 2 + height2 + 4;
	WindowRect2.right = (WindowRect1.right - WindowRect1.left - widht2) / 2 - 2 + widht2 + 4;
	WindowRect2.left = (WindowRect1.right - WindowRect1.left - widht2) / 2 - 2;
	WindowRect2.top = borderHeight / 2 - (captionHeight + menuHeight) - 2;
	MoveWindow(
		hWnd,
		(WindowRect1.right - WindowRect1.left - widht2) / 2 - 2,
		WindowRect2.top,
		widht2 + 4 + 10,
		WindowRect2.bottom - WindowRect2.top + 10,
		0);
	// Todo: WH + 10 hack: original request 640x480 window but somehow receives 650x490, even thought spyxx says it is 640x480
	fullscrn_flag1 = 0;
}

void fullscrn::shutdown()
{
	if (display_changed)
		set_screen_mode(0);
}

int fullscrn::set_screen_mode(int isFullscreen)
{
	int result = isFullscreen;
	if (isFullscreen == screen_mode)
		return result;
	screen_mode = isFullscreen;
	if (isFullscreen)
	{
		if (IsWindowVisible(hWnd))
			GetWindowRect(hWnd, &WindowRect2);
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
	tagRECT Rect{};
	DEVMODEA DevMode{};

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
			SetWindowPos(hWnd, (HWND)-1, 0, 0, Rect.right - Rect.left + 1,
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
	SetWindowPos(hWnd, (HWND)-1, 0, 0, Rect.right - Rect.left + 1, Rect.bottom - Rect.top + 1, 8u);
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
			SetWindowPos(hWnd, (HWND)-1, 0, 0, 0, 0, 0x13u);
	}
	setWindowFlagsDisDlg();
	SetWindowPos(
		hWnd,
		nullptr,
		WindowRect2.left,
		WindowRect2.top,
		WindowRect2.right - WindowRect2.left,
		WindowRect2.bottom - WindowRect2.top,
		0x14u);
	return 0;
}

bool fullscrn::set_menu_mode(int menuEnabled)
{
	BOOL result;

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
	int yPos;
	tagRECT Rect{};

	if (screen_mode)
	{
		GetWindowRect(GetDesktopWindow(), &Rect);
		render::vscreen.XPosition = (Rect.right - render::vscreen.Width - Rect.left) / 2;
		yPos = (Rect.bottom - render::vscreen.Height - Rect.top) / 2;
	}
	else
	{
		render::vscreen.XPosition = 0;
		yPos = GetSystemMetrics(15);
	}
	render::vscreen.YPosition = yPos;
	if (MenuEnabled)
		render::vscreen.YPosition -= GetSystemMetrics(15);
}

void fullscrn::force_redraw()
{
	BYTE1(fullscrn_flag1) |= 0x80u;
}


void fullscrn::center_in(HWND parent, HWND child)
{
	LONG right;
	tagRECT childRect{}, parentRect{}, desktopRect{};

	GetWindowRect(parent, &parentRect);
	GetWindowRect(child, &childRect);
	GetWindowRect(GetDesktopWindow(), &desktopRect);
	if (display_changed)
	{
		desktopRect.bottom = 480;
		desktopRect.left = 0;
		desktopRect.top = 0;
		right = 640;
		desktopRect.right = 640;
		parentRect.left = 0;
		parentRect.top = 0;
		parentRect.right = 640;
		parentRect.bottom = 480;
	}
	else
	{
		right = desktopRect.right;
	}

	int childHeight = childRect.bottom - childRect.top;
	int smthWidth = parentRect.left + (parentRect.right + childRect.left - childRect.right - parentRect.left) / 2;
	int smthHeight = parentRect.top + (parentRect.bottom + childRect.top - childRect.bottom - parentRect.top) / 2;
	if (childRect.right - childRect.left + smthWidth > right)
		smthWidth = right - (childRect.right - childRect.left);
	if (childHeight + smthHeight > desktopRect.bottom)
		smthHeight = desktopRect.bottom - childHeight;
	if (smthWidth < desktopRect.left)
		smthWidth = desktopRect.left;
	if (smthHeight < desktopRect.top)
		smthHeight = desktopRect.top;
	MoveWindow(child, smthWidth, smthHeight, childRect.right - childRect.left, childRect.bottom - childRect.top, 0);
}

int fullscrn::displaychange()
{
	int result = 0;
	if (SmthFullScrnFlag2)
	{
		SmthFullScrnFlag2 = 0;
	}
	else
	{
		if (screen_mode && display_changed)
		{
			display_changed = 0;
			screen_mode = 0;
			setWindowFlagsDisDlg();
			BYTE1(fullscrn_flag1) |= 0x80u;
			InvalidateRect(hWnd, nullptr, 1);
			set_menu_mode(1);
			SetWindowPos(
				hWnd,
				nullptr,
				WindowRect2.left,
				WindowRect2.top,
				WindowRect2.right - WindowRect2.left,
				WindowRect2.bottom - WindowRect2.top,
				0x1Cu);
			result = 1;
		}
		else
		{
			SetWindowPos(
				hWnd,
				nullptr,
				WindowRect2.left,
				WindowRect2.top,
				WindowRect2.right - WindowRect2.left,
				WindowRect2.bottom - WindowRect2.top,
				0x14u);
		}
		center_in(GetDesktopWindow(), hWnd);
	}
	return result;
}

void fullscrn::activate(int flag)
{
	if (screen_mode)
	{
		if (!flag)
		{
			set_screen_mode(0);
			SetWindowPos(hWnd, (HWND)1, 0, 0, 0, 0, 0x13u);
		}
	}
}

void fullscrn::fillRect(int right, int bottom)
{
	// Weird reg usage, should be zero
	int v2 = 0;
	int v3 = 0;

	RECT rc;
	HGDIOBJ brush = CreateSolidBrush(0);
	if (brush)
	{
		HDC dc = winmain::_GetDC(hWnd);
		HGDIOBJ brushHandle = SelectObject(dc, brush);
		if (dc)
		{
			rc.right = right + v2 + 1;
			rc.bottom = bottom + v3 + 1;
			rc.left = v2;
			rc.top = v3;
			FillRect(dc, &rc, static_cast<HBRUSH>(brush));
			ReleaseDC(hWnd, dc);
		}
		SelectObject(dc, brushHandle);
		DeleteObject(brush);
	}
}

unsigned fullscrn::convert_mouse_pos(unsigned int mouseXY)
{
	unsigned __int16 x = mouseXY & 0xffFF - render::vscreen.XPosition;
	unsigned __int16 y = (mouseXY >> 16) - render::vscreen.YPosition;
	return x | y << 16;
}

void fullscrn::getminmaxinfo(MINMAXINFO* maxMin)
{
	maxMin->ptMaxSize.x = WindowRect2.right - WindowRect2.left;
	maxMin->ptMaxSize.y = WindowRect2.bottom - WindowRect2.top;
	maxMin->ptMaxPosition.x = WindowRect2.left;
	maxMin->ptMaxPosition.y = WindowRect2.top;
}

void fullscrn::paint()
{
	int menuHeight;
	if (screen_mode)
	{
		if ((fullscrn_flag1 & 0x8000) == 0 && fullscrn_flag1)
		{
			if (fullscrn_flag1 & 1)
			{
				menuHeight = GetSystemMetrics(SM_CYMENU);
				fillRect(WindowRect1.right - 1, menuHeight);
			}
		}
		else
		{
			if (MenuEnabled)
				menuHeight = GetSystemMetrics(SM_CYMENU);
			else
				menuHeight = 0;
			fillRect(WindowRect1.right, menuHeight + WindowRect1.bottom);
		}
	}
	render::paint();
	fullscrn_flag1 = 0;
}
