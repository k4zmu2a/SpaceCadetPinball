#pragma once
#include "gdrv.h"

class winmain
{
public:
	static int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
	static LRESULT CALLBACK message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void memalloc_failure();
	static int ProcessWindowMessages();
	static int check_expiration_date();
	static HDC _GetDC(HWND hWnd);
	static int a_dialog(HINSTANCE hInstance, HWND hWnd);
private:
	static int iFrostUniqueMsg, return_value, bQuit, DispFrameRate, DispGRhistory;
	static int has_focus, single_step, mouse_down, last_mouse_x, last_mouse_y, no_time_loss;
	static DWORD then, now;
	static gdrv_bitmap8 gfr_display;

	static HDC _BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint);
};
