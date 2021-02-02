#pragma once

struct LOGPALETTEx256;

struct splash_struct
{
	HWND WindowHandle;
	HPALETTE Palette;
	HBITMAP Bitmap;
	HDC DrawingContext;
	int Width;
	int Height;
	int CenterX;
	int CenterY;
	char BmpName1[200];
	char BmpName2[200];
};


class splash
{
public:
	static splash_struct* splash_screen(HINSTANCE hInstance, LPCSTR bmpName1, LPCSTR bmpName2);
	static void splash_bitmap_setup(splash_struct* splashStruct);
	static HBITMAP load_title_bitmap(HMODULE hModule, HDC hdc, LPCSTR lpName, UINT iStart, int iEnd,
	                                 HPALETTE* palettePtr);
	static HPALETTE splash_init_palette(LOGPALETTE* plpal);
	static void splash_paint(splash_struct* splashStruct, HDC dc);
	static void splash_destroy(splash_struct* splashStruct);
	static void splash_hide(splash_struct* splashStruct);
	static LRESULT __stdcall splash_message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
private:
	static HINSTANCE HInstance;
	static HGDIOBJ OriginalDcBitmap;
};
