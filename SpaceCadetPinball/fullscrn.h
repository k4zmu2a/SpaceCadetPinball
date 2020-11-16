#pragma once
#include "maths.h"

#define BYTEn(x, n)   (*((unsigned char*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)   BYTEn(x,  2)

class fullscrn
{
public:

	static int screen_mode;
	static HWND hWnd;
	static tagRECT WindowRect1, WindowRect2;
	static rectangle_type WHRect;
	static int fullscrn_flag1;
	static int display_changed;
	static int ChangeDisplay, SmthFullScrnFlag2;
	static int trick;

	static void init(int width, int height, int isFullscreen, HWND winHandle, HMENU menuHandle, int changeDisplay);
	static void shutdown();
	static int set_screen_mode(int isFullscreen);
	static void force_redraw();
	static void center_in(HWND parent, HWND child);
	static int displaychange();
	static void activate(int flag);
	static int convert_mouse_pos(unsigned int mouseXY);
	static void getminmaxinfo(MINMAXINFO* maxMin);
	static void paint();
private :
	static int MenuEnabled;
	static HMENU MenuHandle;

	static void GetWindowCenter();
	static int disableWindowFlagsDisDlg();
	static int setWindowFlagsDisDlg();
	static int enableFullscreen();
	static int disableFullscreen();
	static bool set_menu_mode(int menuEnabled);
	static void fillRect(int right, int bottom);
};
