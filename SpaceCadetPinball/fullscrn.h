#pragma once
#include "maths.h"

#define BYTEn(x, n)   (*((unsigned char*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)   BYTEn(x,  2)


struct resolution_info
{
	__int16 ScreenWidth;
	__int16 ScreenHeight;
	__int16 TableWidth;
	__int16 TableHeight;
	__int16 ResolutionMenuId;
};

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
	static const resolution_info resolution_array[3];

	static void init(int width, int height, int isFullscreen, HWND winHandle, HMENU menuHandle, int changeDisplay);
	static void shutdown();
	static int set_screen_mode(int isFullscreen);
	static void force_redraw();
	static void center_in(HWND parent, HWND child);
	static int displaychange();
	static void activate(int flag);
	static unsigned convert_mouse_pos(unsigned int mouseXY);
	static void getminmaxinfo(MINMAXINFO* maxMin);
	static void paint();
	static bool set_menu_mode(int menuEnabled);
	static int GetResolution();
	static void SetResolution(int resolution);
	static int GetMaxResolution();
	static void SetMaxResolution(int resolution);
	static int get_max_supported_resolution();
	static int get_screen_resolution();
private :
	static int MenuEnabled;
	static HMENU MenuHandle;
	static int resolution;
	static int maxResolution;

	static void GetWindowCenter();
	static int disableWindowFlagsDisDlg();
	static int setWindowFlagsDisDlg();
	static int enableFullscreen();
	static int disableFullscreen();
	static void fillRect(int right, int bottom);
};
