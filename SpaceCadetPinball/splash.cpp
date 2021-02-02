#include "pch.h"
#include "splash.h"


#include "memory.h"
#include "pinball.h"

HINSTANCE splash::HInstance;
HGDIOBJ splash::OriginalDcBitmap = nullptr;

splash_struct* splash::splash_screen(HINSTANCE hInstance, LPCSTR bmpName1, LPCSTR bmpName2)
{
	WNDCLASSA WndClass{};
	tagRECT Rect{};

	auto splashStruct = memory::allocate<splash_struct>();
	if (!splashStruct)
		return nullptr;

	lstrcpyA(splashStruct->BmpName1, bmpName1);
	lstrcpyA(splashStruct->BmpName2, bmpName2);
	if (!HInstance)
	{
		HInstance = hInstance;
		WndClass.style = 0;
		WndClass.lpfnWndProc = splash_message_handler;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 4;
		WndClass.hInstance = hInstance;
		WndClass.hIcon = nullptr;
		WndClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
		WndClass.hbrBackground = nullptr;
		WndClass.lpszMenuName = pinball::WindowName;
		WndClass.lpszClassName = "3DPB_SPLASH_CLASS";
		RegisterClassA(&WndClass);
	}
	splashStruct->Bitmap = nullptr;
	HWND windowHandle = CreateWindowExA(0, "3DPB_SPLASH_CLASS", pinball::WindowName, 0x80000000, -10, -10, 1, 1,
	                                    nullptr, nullptr, HInstance, nullptr);
	splashStruct->WindowHandle = windowHandle;
	if (!windowHandle)
	{
		memory::free(splashStruct);
		return nullptr;
	}

	SetWindowLongPtrA(windowHandle, -21, reinterpret_cast<LONG_PTR>(splashStruct));
	GetWindowRect(GetDesktopWindow(), &Rect);
	splash_bitmap_setup(splashStruct);
	//MoveWindow(splashStruct->WindowHandle, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, 0);

	/*Mod - less intrusive splash*/
	auto centerX = (Rect.right - Rect.left - splashStruct->Width) / 2;
	auto CenterY = (Rect.bottom - Rect.top - splashStruct->Height) / 2;
	MoveWindow(splashStruct->WindowHandle, centerX, CenterY, splashStruct->Width, splashStruct->Height, 0);

	ShowWindow(splashStruct->WindowHandle, 8);
	SetFocus(splashStruct->WindowHandle);
	UpdateWindow(splashStruct->WindowHandle);
	return splashStruct;
}

void splash::splash_bitmap_setup(splash_struct* splashStruct)
{
	HBITMAP bmpHandle2;
	BITMAP bmp{};

	HBITMAP bmpHandle1 = nullptr;
	HDC desktopDC = GetDC(GetDesktopWindow());
	if (desktopDC)
	{
		splashStruct->DrawingContext = CreateCompatibleDC(desktopDC);
		bmpHandle1 = CreateCompatibleBitmap(desktopDC, 10, 10);
		ReleaseDC(splashStruct->WindowHandle, desktopDC);
		if (bmpHandle1)
		{
			if (splashStruct->DrawingContext)
			{
				OriginalDcBitmap = SelectObject(splashStruct->DrawingContext, bmpHandle1);
				if ((GetDeviceCaps(splashStruct->DrawingContext, RASTERCAPS) & RC_PALETTE) != 0
					|| GetDeviceCaps(splashStruct->DrawingContext, NUMCOLORS) >= 256)
				{
					bmpHandle2 = load_title_bitmap(HInstance, splashStruct->DrawingContext, splashStruct->BmpName1, 10,
					                               236, &splashStruct->Palette);
				}
				else
				{
					bmpHandle2 = LoadBitmapA(HInstance, splashStruct->BmpName2);
					splashStruct->Palette = nullptr;
				}
				splashStruct->Bitmap = bmpHandle2;

				if (bmpHandle2)
				{
					SelectObject(splashStruct->DrawingContext, bmpHandle2);
					DeleteObject(bmpHandle1);
					GetObjectA(splashStruct->Bitmap, sizeof(BITMAP), &bmp);
					splashStruct->Width = bmp.bmWidth;
					splashStruct->Height = bmp.bmHeight;
					return;
				}
			}
		}
		else
		{
			GetLastError();
		}
	}

	if (splashStruct->Palette)
		DeleteObject(splashStruct->Palette);
	if (splashStruct->WindowHandle)
		DestroyWindow(splashStruct->WindowHandle);
	if (bmpHandle1)
		DeleteObject(bmpHandle1);
	if (splashStruct->DrawingContext)
		DeleteDC(splashStruct->DrawingContext);
	splashStruct->Bitmap = nullptr;
}

HBITMAP splash::load_title_bitmap(HMODULE hModule, HDC hdc, LPCSTR lpName, UINT iStart, int iEnd, HPALETTE* palettePtr)
{
	LOGPALETTEx256 plpal;

	auto resH = FindResourceA(hModule, lpName, RT_BITMAP);
	if (!resH)
		return nullptr;
	auto resHGlobal = LoadResource(hModule, resH);
	if (!resHGlobal)
		return nullptr;

	auto bmp = static_cast<BITMAPINFO*>(LockResource(resHGlobal));
	int numColors = bmp->bmiHeader.biClrUsed;
	if (!numColors)
		numColors = 1 << LOBYTE(bmp->bmiHeader.biBitCount);
	if (bmp->bmiHeader.biBitCount > 4u)
	{
		*palettePtr = splash_init_palette(&plpal);
		if (*palettePtr)
		{
			int cEntries = 0;
			if (iEnd > 0)
			{
				auto dst = &plpal.palPalEntry[iStart];
				auto src = &bmp->bmiColors[0];
				for (; cEntries < iEnd && cEntries < numColors; ++cEntries)
				{
					dst->peRed = src->rgbRed;
					dst->peGreen = src->rgbGreen;
					dst->peBlue = src->rgbBlue;
					dst->peFlags = 4;
					src++;
					dst++;
				}
			}
			SetPaletteEntries(*palettePtr, iStart, cEntries, &plpal.palPalEntry[iStart]);
			SelectPalette(hdc, *palettePtr, 0);
			RealizePalette(hdc);
		}
	}

	auto resBmp = CreateDIBitmap(hdc, &bmp->bmiHeader, 4u, &bmp->bmiColors[numColors], bmp, 0);
	FreeResource(resHGlobal);
	return resBmp;
}

HPALETTE splash::splash_init_palette(LOGPALETTE* plpal)
{
	plpal->palVersion = 768;
	plpal->palNumEntries = 256;
	auto hPalette = CreatePalette(static_cast<const LOGPALETTE*>(plpal));
	auto dc = GetDC(GetDesktopWindow());
	GetDeviceCaps(dc, RASTERCAPS);
	if (GetDeviceCaps(dc, SIZEPALETTE) != 256)
	{
		if (hPalette)
			DeleteObject(hPalette);
		ReleaseDC(GetDesktopWindow(), dc);
		return nullptr;
	}

	SetSystemPaletteUse(dc, 2u);
	SetSystemPaletteUse(dc, 1u);
	auto hPal = SelectPalette(dc, hPalette, 0);
	RealizePalette(dc);
	SelectPalette(dc, hPal, 0);
	RealizePalette(dc);
	GetSystemPaletteEntries(dc, 0, 256u, plpal->palPalEntry);
	ReleaseDC(GetDesktopWindow(), dc);

	auto dst = &plpal->palPalEntry[0];
	for (auto index = 256; index; --index)
	{
		dst->peFlags = 0;
		dst++;
	}

	dst = &plpal->palPalEntry[10];
	for (auto index = 10; index < 246; ++index)
	{
		dst->peRed = index;
		dst->peGreen = index;
		dst->peBlue = index;
		dst->peFlags = 4;
		dst++;
	}
	ResizePalette(hPalette, 256u);
	SetPaletteEntries(hPalette, 0, 256u, plpal->palPalEntry);
	return hPalette;
}

void splash::splash_paint(splash_struct* splashStruct, HDC dc)
{
	tagRECT Rect{};

	if (splashStruct->Bitmap)
	{
		GetWindowRect(GetDesktopWindow(), &Rect);
		splashStruct->CenterX = (Rect.right - Rect.left - splashStruct->Width) / 2;
		splashStruct->CenterY = (Rect.bottom - Rect.top - splashStruct->Height) / 2;
		SelectPalette(dc, splashStruct->Palette, 0);
		RealizePalette(dc);
		SelectPalette(splashStruct->DrawingContext, splashStruct->Palette, 0);
		RealizePalette(splashStruct->DrawingContext);
		/*BitBlt(dc, splashStruct->CenterX, splashStruct->CenterY, splashStruct->Width,
		       splashStruct->Height, splashStruct->DrawingContext, 0, 0, SRCCOPY);*/

		/*Mod - less intrusive splash*/
		BitBlt(dc, 0, 0, splashStruct->Width, splashStruct->Height,
		       splashStruct->DrawingContext, 0, 0, SRCCOPY);
	}
}

void splash::splash_destroy(splash_struct* splashStruct)
{
	if (splashStruct)
	{
		if (splashStruct->WindowHandle)
		{
			DestroyWindow(splashStruct->WindowHandle);
			splashStruct->WindowHandle = nullptr;

			if (splashStruct->Palette)
				DeleteObject(splashStruct->Palette);
			splashStruct->Palette = nullptr;

			if (splashStruct->DrawingContext)
			{
				if (OriginalDcBitmap)
					SelectObject(splashStruct->DrawingContext, OriginalDcBitmap);
				DeleteDC(splashStruct->DrawingContext);
			}
			if (splashStruct->Bitmap)
				DeleteObject(splashStruct->Bitmap);
		}
		memory::free(splashStruct);
	}
	if (HInstance)
	{
		UnregisterClassA("3DPB_SPLASH_CLASS", HInstance);
		HInstance = nullptr;
	}
}

void splash::splash_hide(splash_struct* splashStruct)
{
	if (splashStruct && splashStruct->WindowHandle)
	{
		HDC dc = GetDC(splashStruct->WindowHandle);
		BitBlt(dc, 0, 0, splashStruct->CenterX, splashStruct->CenterY, dc, 0, 0, 0x42u);
		ReleaseDC(splashStruct->WindowHandle, dc);
	}
}

LRESULT splash::splash_message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	tagPAINTSTRUCT Paint{};

	switch (Msg)
	{
	case WM_PAINT:
		{
			auto splashStruct = reinterpret_cast<splash_struct*>(GetWindowLongPtrA(hWnd, -21));
			BeginPaint(hWnd, &Paint);
			EndPaint(hWnd, &Paint);
			auto dc = GetDC(hWnd);
			if (dc && splashStruct)
			{
				BitBlt(dc, 0, 0, 10000, 10000, dc, 0, 0, BLACKNESS);
				splash_paint(splashStruct, dc);
			}
			ReleaseDC(hWnd, dc);
			break;
		}
	case WM_ERASEBKGND:
		break;
	default:
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	return 0;
}
