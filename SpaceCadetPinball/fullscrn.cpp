#include "pch.h"
#include "fullscrn.h"


#include "options.h"
#include "pb.h"
#include "render.h"
#include "winmain.h"


int fullscrn::screen_mode;
HWND fullscrn::hWnd;
tagRECT fullscrn::WindowRect1, fullscrn::WindowRect2;
rectangle_type fullscrn::WHRect;
int fullscrn::fullscrn_flag1;
int fullscrn::display_changed;
int fullscrn::ChangeDisplay, fullscrn::ignoreNextDisplayChangeFg;
int fullscrn::trick = 1;
int fullscrn::MenuEnabled;
HMENU fullscrn::MenuHandle;
int fullscrn::resolution = 0;
int fullscrn::maxResolution = 0;
const resolution_info fullscrn::resolution_array[3] =
{
	{640, 480, 600, 416, 501},
	{800, 600, 752, 520, 502},
	{1024, 768, 960, 666, 503},
};
float fullscrn::ScaleX = 1;
float fullscrn::ScaleY = 1;
float fullscrn::OffsetX = 0;
float fullscrn::OffsetY = 0;

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

	/*RECT client{0,0,width,height};
	AdjustWindowRect(&client, winmain::WndStyle, true);*/
	MoveWindow(
		hWnd,
		(WindowRect1.right - WindowRect1.left - widht2) / 2 - 2,
		WindowRect2.top,
		WindowRect2.right - WindowRect2.left + 10,
		WindowRect2.bottom - WindowRect2.top + 10,
		0);
	// Todo: WH + 10 hack: original request 640x480 window but somehow receives 650x490, even thought spyxx says it is 640x480
	fullscrn_flag1 = 0;

	window_size_changed();
	assertm(ScaleX == 1 && ScaleY == 1, "Wrong default client size");
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
	return SetWindowLongA(hWnd, -16, style & ~(WS_CAPTION | WS_THICKFRAME));
}

int fullscrn::setWindowFlagsDisDlg()
{
	int style = GetWindowLongA(hWnd, -16);
	return SetWindowLongA(hWnd, -16, style | WS_CAPTION | WS_THICKFRAME);
}

int fullscrn::enableFullscreen()
{
	tagRECT Rect{};
	DEVMODEA DevMode{};

	if (ChangeDisplay && !display_changed)
	{
		DevMode.dmSize = sizeof DevMode;
		DevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		DevMode.dmPelsWidth = resolution_array[resolution].ScreenWidth;
		DevMode.dmPelsHeight = resolution_array[resolution].ScreenHeight;
		DevMode.dmBitsPerPel = 32;
		disableWindowFlagsDisDlg();
		if (trick)
		{
			GetWindowRect(GetDesktopWindow(), &Rect);
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, Rect.right - Rect.left + 1,
			             Rect.bottom - Rect.top + 1, SWP_NOREDRAW);
		}
		ignoreNextDisplayChangeFg = 1;
		LONG changeDispResult = ChangeDisplaySettingsA(&DevMode, CDS_FULLSCREEN);
		if (changeDispResult == DISP_CHANGE_RESTART)
		{
			DevMode.dmFields &= ~DM_BITSPERPEL;
			ignoreNextDisplayChangeFg = 1;
			changeDispResult = ChangeDisplaySettingsA(&DevMode, CDS_FULLSCREEN);
		}
		display_changed = changeDispResult == DISP_CHANGE_SUCCESSFUL;
		if (display_changed)
			return 1;
	}
	GetWindowRect(GetDesktopWindow(), &Rect);
	disableWindowFlagsDisDlg();
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, Rect.right - Rect.left + 1, Rect.bottom - Rect.top + 1, SWP_NOREDRAW);
	return 0;
}

int fullscrn::disableFullscreen()
{
	if (display_changed)
	{
		display_changed = 0;
		ignoreNextDisplayChangeFg = 1;
		ChangeDisplaySettingsA(nullptr, CDS_FULLSCREEN);
		if (trick)
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	setWindowFlagsDisDlg();
	SetWindowPos(
		hWnd,
		HWND_TOP,
		WindowRect2.left,
		WindowRect2.top,
		WindowRect2.right - WindowRect2.left,
		WindowRect2.bottom - WindowRect2.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
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
	if (ignoreNextDisplayChangeFg)
	{
		ignoreNextDisplayChangeFg = 0;
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
				HWND_TOP,
				WindowRect2.left,
				WindowRect2.top,
				WindowRect2.right - WindowRect2.left,
				WindowRect2.bottom - WindowRect2.top,
				SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
			result = 1;
		}
		else
		{
			SetWindowPos(
				hWnd,
				HWND_TOP,
				WindowRect2.left,
				WindowRect2.top,
				WindowRect2.right - WindowRect2.left,
				WindowRect2.bottom - WindowRect2.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
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
			SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}
}

void fullscrn::fillRect(int right, int bottom, int left, int top)
{
	RECT rc;
	auto brush = CreateSolidBrush(0);
	if (brush)
	{
		auto dc = winmain::_GetDC(hWnd);
		auto prevBrush = SelectObject(dc, brush);
		if (dc)
		{
			rc.right = left + right + 1;
			rc.bottom = top + bottom + 1;
			rc.left = left;
			rc.top = top;
			FillRect(dc, &rc, brush);
			ReleaseDC(hWnd, dc);
		}
		SelectObject(dc, prevBrush);
		DeleteObject(brush);
	}
}

unsigned fullscrn::convert_mouse_pos(unsigned int mouseXY)
{
	uint16_t x = mouseXY & 0xffFF - render::vscreen.XPosition;
	uint16_t y = (mouseXY >> 16) - render::vscreen.YPosition;
	return x | y << 16;
}

void fullscrn::getminmaxinfo(MINMAXINFO* maxMin)
{
	/*Block down-scaling lower than min resolution*/
	maxMin->ptMinTrackSize = POINT
	{
		resolution_array[0].ScreenWidth / 2,
		resolution_array[0].ScreenHeight / 2
	};
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
				fillRect(WindowRect1.right - 1, menuHeight, 0, 0);
			}
		}
		else
		{
			if (MenuEnabled)
				menuHeight = GetSystemMetrics(SM_CYMENU);
			else
				menuHeight = 0;
			fillRect(WindowRect1.right, menuHeight + WindowRect1.bottom, 0, 0);
		}
	}
	render::paint();
	fullscrn_flag1 = 0;
}

int fullscrn::GetResolution()
{
	return resolution;
}

void fullscrn::SetResolution(int resolution)
{
	if (!pb::FullTiltMode)
		resolution = 0;
	assertm(resolution >= 0 && resolution <= 2, "Resolution value out of bounds");
	fullscrn::resolution = resolution;
}

int fullscrn::GetMaxResolution()
{
	return maxResolution;
}

void fullscrn::SetMaxResolution(int resolution)
{
	assertm(resolution >= 0 && resolution <= 2, "Resolution value out of bounds");
	maxResolution = resolution;
}

int fullscrn::get_max_supported_resolution()
{
	if (!pb::FullTiltMode)
		return 0;

	auto resolutionWH = get_screen_resolution();
	auto width = LOWORD(resolutionWH);
	auto height = HIWORD(resolutionWH);
	auto result = 0;

	for (auto index = 1; index < 3; ++index)
	{
		auto resPtr = &resolution_array[index];
		if (resPtr->ScreenWidth <= width && resPtr->ScreenHeight <= height)
			result = index;
	}
	return result;
}

int fullscrn::get_screen_resolution()
{
	auto height = static_cast<uint16_t>(GetSystemMetrics(SM_CYSCREEN));
	return static_cast<uint16_t>(GetSystemMetrics(SM_CXSCREEN)) | (height << 16);
}

void fullscrn::window_size_changed()
{
	/*No scaling in fullscreen mode*/
	if (display_changed)
	{
		ScaleY = ScaleX = 1;
		OffsetX = OffsetY = 0;
		return;
	}

	RECT client{};
	GetClientRect(hWnd, &client);
	auto res = &resolution_array[resolution];
	ScaleX = static_cast<float>(client.right) / res->TableWidth;
	ScaleY = static_cast<float>(client.bottom) / res->TableHeight;
	OffsetX = OffsetY = 0;

	if (options::Options.UniformScaling)
	{
		ScaleY = ScaleX = min(ScaleX, ScaleY);
		OffsetX = floor((client.right - res->TableWidth * ScaleX) / 2);
		OffsetY = floor((client.bottom - res->TableHeight * ScaleY) / 2);
		auto dc = GetDC(hWnd);
		if (dc)
		{
			BitBlt(dc, 0, 0, client.right, client.bottom, dc, 0, 0, BLACKNESS);
			ReleaseDC(hWnd, dc);
		}
	}
}
