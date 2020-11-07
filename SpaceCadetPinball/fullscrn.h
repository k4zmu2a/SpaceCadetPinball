#pragma once

#define BYTEn(x, n)   (*((unsigned char*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)   BYTEn(x,  2)

class fullscrn
{
public:

	static int screen_mode;
	static HWND hWnd;
	static tagRECT PubRect1;
	static int fullscrn_flag1;
	static int display_changed;
	static int ChangeDisplay, SmthFullScrnFlag2;
	static int trick;

	static int set_screen_mode(int isFullscreen);
	static void force_redraw();
private :
	static int MenuEnabled;
	static HMENU MenuHandle;
	static int xDest, yDest;
	
	static void GetWindowCenter();	
	static int disableWindowFlagsDisDlg();
	static int setWindowFlagsDisDlg();
	static int enableFullscreen();
	static int disableFullscreen();	
	static bool set_menu_mode(int menuEnabled);
};
