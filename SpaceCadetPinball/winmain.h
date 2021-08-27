#pragma once
#include "gdrv.h"

class winmain
{
public:
	static const DWORD WndStyle = WS_GROUP | WS_SYSMENU | WS_DLGFRAME | WS_BORDER | WS_MAXIMIZE | WS_CLIPCHILDREN |
		WS_THICKFRAME | WS_MAXIMIZEBOX;
	static char DatFileName[300];
	static int single_step;
	static HINSTANCE hinst;
	static HWND hwnd_frame;

	static int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
	static LRESULT CALLBACK message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static int event_handler(const SDL_Event* event);
	static void memalloc_failure();
	static int ProcessWindowMessages();
	static int check_expiration_date();
	static HDC _GetDC(HWND hWnd);
	static int a_dialog(HINSTANCE hInstance, HWND hWnd);
	static void end_pause();
	static void new_game();
	static void pause();
	static void help_introduction(HINSTANCE a1, HWND a2);
	static void Restart();
private:
	static int return_value, bQuit, DispFrameRate, DispGRhistory, activated;
	static int has_focus, mouse_down, last_mouse_x, last_mouse_y, no_time_loss;
	static DWORD then, now;
	static UINT iFrostUniqueMsg;
	static gdrv_bitmap8 gfr_display;
	static HCURSOR mouse_hsave;
	static bool restart;
	static SDL_Window* MainWindow;

	static HDC _BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint);
};
