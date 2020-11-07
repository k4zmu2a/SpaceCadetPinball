#pragma once

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
	static int iFrostUniqueMsg, return_value , bQuit;

	static HDC _BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint);
};
