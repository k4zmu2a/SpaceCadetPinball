#include "pch.h"
#include "winmain.h"
#include "memory.h"
#include "pinball.h"
#include "options.h"

int iFrostUniqueMsg;

//HWND, UINT, WPARAM, LPARAM
//typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM LPARAM)
{
	return 0;
}

int check_expiration_date()
{
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	memory::init(winmain_memalloc_failure);
	++memory::critical_allocation;
	auto optionsRegPath = pinball::get_rc_string(165, 0);
	options::path_init(optionsRegPath);
	auto regSpaceCadet = pinball::get_rc_string(166, 0);

	if (options::get_int(regSpaceCadet, "Table Version", 1) <= 1)
	{
		auto tmpBuf = memory::allocate(0x1F4u);
		if (!tmpBuf)
		{
			options::path_uninit();
			return 0;
		}

		options::set_int(regSpaceCadet, "Table Version", 1u);
		GetModuleFileNameA(pinball::hinst, tmpBuf, 0x1F4u);
		options::set_string(regSpaceCadet, "Table Exe", tmpBuf);
		options::set_string(regSpaceCadet, "Table Name", pinball::get_rc_string(169, 0));
		options::set_string(nullptr, "Last Table Played", regSpaceCadet);
		memory::free(static_cast<void*>(tmpBuf));
		tmpBuf = memory::allocate(0x1F4u);
		if (tmpBuf)
		{
			auto tmpBuf2 = memory::allocate(0x1F4u);
			if (tmpBuf2)
			{
				char Buffer[40];
				for (int i = 0; i < 32700; ++i)
				{
					sprintf_s(Buffer, "Table%d", i);
					options::get_string(nullptr, Buffer, tmpBuf, pinball::WindowName, 500);
					if (!*tmpBuf)
						break;
					options::get_string(tmpBuf, "Table Name", tmpBuf2, pinball::WindowName, 500);
					if (!lstrcmpA(tmpBuf2, pinball::get_rc_string(169, 0)))
						goto LABEL_15;
					if (!*tmpBuf2)
						break;
				}
				options::set_string(nullptr, Buffer, regSpaceCadet);
			LABEL_15:
				memory::free(tmpBuf2);
			}
			memory::free(tmpBuf);
		}
	}
	else
	{
		auto tmpBuf = memory::allocate(0x1F4u);
		if (!tmpBuf)
		{
			options::path_uninit();
			return 0;
		}
		options::get_string(regSpaceCadet, "Shell Exe", tmpBuf, pinball::WindowName, 500);
		auto execRes = WinExec(tmpBuf, 5u);
		memory::free(tmpBuf);
		if (execRes >= 32)
		{
			options::path_uninit();
			return 0;
		}
	}
	--memory::critical_allocation;

	pinball::quickFlag = strstr(lpCmdLine, "-quick") != 0;
	pinball::hinst = hInstance;
	options::get_string(regSpaceCadet, "Pinball Data", pinball::DatFileName, pinball::get_rc_string(168, 0), 300);

	iFrostUniqueMsg = RegisterWindowMessageA("PinballThemeSwitcherUniqueMsgString");
	auto windowHandle= FindWindowA(pinball::get_rc_string(167, 0), nullptr);
	if (windowHandle)
	{
		SendMessageA(windowHandle, iFrostUniqueMsg, 0, 0);
		return 0;
	}

	if (check_expiration_date())
		return 0;

	INITCOMMONCONTROLSEX picce{};
	picce.dwSize = 8;
	picce.dwICC = 5885;
	InitCommonControlsEx(&picce);

	WNDCLASSA WndClass;
	WndClass.style = 4104;
	WndClass.lpfnWndProc = message_handler;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIconA(hInstance, "ICON_1");
	WndClass.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
	WndClass.hbrBackground = (HBRUSH)16;
	WndClass.lpszMenuName = "MENU_1";
	WndClass.lpszClassName = pinball::get_rc_string(167, 0);
	//auto tmpBuf = splash_screen((int)hInstance, "splash_bitmap", "splash_bitmap");
	RegisterClassA(&WndClass);
}

void winmain_memalloc_failure()
{
	/*midi_music_stop();
	Sound_Close();
	gdrv_uninit();*/
	char* caption = pinball::get_rc_string(170, 0);
	char* text = pinball::get_rc_string(179, 0);
	MessageBoxA(nullptr, text, caption, 0x2030u);
	_exit(1);
}
