#include "pch.h"
#include "winmain.h"

#include "fullscrn.h"
#include "memory.h"
#include "pinball.h"
#include "options.h"
#include "pb.h"
#include "Sound.h"

int winmain::iFrostUniqueMsg, winmain::return_value = 0, winmain::bQuit = 0;
DWORD winmain::then, winmain::now;
gdrv_bitmap8 winmain::gfr_display{};
int winmain::DispFrameRate = 1, winmain::DispGRhistory = 1, winmain::single_step = 0;
int winmain::has_focus = 1, winmain::last_mouse_x, winmain::last_mouse_y, winmain::mouse_down, winmain::no_time_loss;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return winmain::WinMain(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

int winmain::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	memory::init(memalloc_failure);
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

	pinball::quickFlag = strstr(lpCmdLine, "-quick") != nullptr;
	pinball::hinst = hInstance;
	options::get_string(regSpaceCadet, "Pinball Data", pinball::DatFileName, pinball::get_rc_string(168, 0), 300);

	iFrostUniqueMsg = RegisterWindowMessageA("PinballThemeSwitcherUniqueMsgString");
	auto windowHandle = FindWindowA(pinball::get_rc_string(167, 0), nullptr);
	if (windowHandle)
	{
		SendMessageA(windowHandle, iFrostUniqueMsg, 0, 0);
		return 0;
	}

	if (check_expiration_date())
		return 0;

	INITCOMMONCONTROLSEX picce;
	picce.dwSize = 8;
	picce.dwICC = 5885;
	InitCommonControlsEx(&picce);

	auto windowClass = pinball::get_rc_string(167, 0);
	WNDCLASSA WndClass{};
	WndClass.style = 4104;
	WndClass.lpfnWndProc = message_handler;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIconA(hInstance, "ICON_1");
	WndClass.hCursor = LoadCursorA(nullptr, (LPCSTR)0x7F00);
	WndClass.hbrBackground = (HBRUSH)16;
	WndClass.lpszMenuName = "MENU_1";
	WndClass.lpszClassName = windowClass;
	//auto tmpBuf = splash_screen((int)hInstance, "splash_bitmap", "splash_bitmap"); // No splash for now
	RegisterClassA(&WndClass);

	pinball::FindShiftKeys();

	char windowName[40];
	lstrcpyA(windowName, pinball::get_rc_string(38, 0));
	windowHandle = CreateWindowExA(0, windowClass, windowName, 0x3CA0000u, 0, 0, 640, 480, nullptr, nullptr, hInstance,
	                               nullptr);
	pinball::hwnd_frame = windowHandle;
	if (!windowHandle)
	{
		PostQuitMessage(0);
		return 0;
	}

	auto menuHandle = GetMenu(windowHandle);
	options::init(menuHandle);
	pb::reset_table();
	pb::firsttime_setup();

	if (strstr(lpCmdLine, "-fullscreen"))
	{
		options::Options.FullScreen = 1;
		options::menu_check(0x193u, 1);
	}

	ShowWindow(pinball::hwnd_frame, nShowCmd);
	fullscrn::set_screen_mode(options::Options.FullScreen);
	UpdateWindow(pinball::hwnd_frame);

	/*if (tmpBuf) //Close splash
	{
		splash_hide(tmpBuf);
		splash_destroy(tmpBuf);
	}*/

	pinball::adjust_priority(options::Options.PriorityAdj);
	const auto startTime = timeGetTime();
	MSG wndMessage{};
	while (timeGetTime() >= startTime && timeGetTime() - startTime < 2000)
		PeekMessageA(&wndMessage, pinball::hwnd_frame, 0, 0, 1u);

	if (strstr(lpCmdLine, "-demo"))
		pb::toggle_demo();
	else
		pb::replay_level(0);

	DWORD someTimeCounter = 300u, prevTime = 0u;
	then = timeGetTime();
	while (true)
	{
		if (!someTimeCounter)
		{
			someTimeCounter = 300;
			if (DispFrameRate)
			{
				auto curTime = timeGetTime();
				if (prevTime)
				{
					char buf[60];
					sprintf_s(buf, "Frames/sec = %02.02f", 300.0f / (static_cast<float>(curTime - prevTime) * 0.001f));
					SetWindowTextA(pinball::hwnd_frame, buf);

					if (DispGRhistory)
					{
						if (!gfr_display.BmpBufPtr1)
						{
							auto plt = static_cast<PALETTEENTRY*>(malloc(1024u));
							auto pltPtr = &plt[10];
							for (int i1 = 0, i2 = 0; i1 < 256 - 10; ++i1, i2 += 8)
							{
								unsigned char blue = i2, redGreen = i2;
								if (i2 > 255)
								{
									blue = 255;
									redGreen = i1;
								}

								*pltPtr++ = {redGreen, redGreen, blue};
							}
							gdrv::display_palette(plt);
							free(plt);
							gdrv::create_bitmap(&gfr_display, 400, 15);
						}

						gdrv::blit(&gfr_display, 0, 0, 0, 0, 300, 10);
						gdrv::fill_bitmap(&gfr_display, 300, 10, 0, 0, 0);
					}
				}
				prevTime = curTime;
			}
			else
			{
				prevTime = 0;
			}
		}

		Sound::Idle();
		if (!ProcessWindowMessages() || bQuit)
			break;

		if (has_focus)
		{
			if (mouse_down)
			{
				now = timeGetTime();
				if (now - then >= 2)
				{
					POINT Point;
					GetCursorPos(&Point);
					pb::ballset(last_mouse_x - Point.x, Point.y - last_mouse_y);
					SetCursorPos(last_mouse_x, last_mouse_y);
				}
			}
			if (!single_step)
			{
				auto curTime = timeGetTime();
				now = curTime;
				if (no_time_loss)
				{
					then = curTime;
					no_time_loss = 0;
				}

				if (curTime == then)
				{
					Sleep(8u);
				}
				else if (pb::frame(curTime - then))
				{
					if (gfr_display.BmpBufPtr1)
					{
						auto deltaT = now - then + 10;
						auto fillChar = static_cast<char>(deltaT);
						if (deltaT > 236)
						{
							fillChar = -7;
						}
						gdrv::fill_bitmap(&gfr_display, 1, 10, 299u - someTimeCounter, 0, fillChar);
					}
					--someTimeCounter;
					then = now;
				}
			}
		}
	}

	return return_value;
}

LRESULT CALLBACK winmain::message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

int winmain::ProcessWindowMessages()
{
	MSG Msg{}; // [esp+8h] [ebp-1Ch]

	if (has_focus && !single_step)
	{
		while (PeekMessageA(&Msg, nullptr, 0, 0, 1u))
		{
			TranslateMessage(&Msg);
			DispatchMessageA(&Msg);
			if (Msg.message == 18)
			{
				return_value = Msg.wParam;
				return 0;
			}
		}
		return 1;
	}
	GetMessageA(&Msg, pinball::hwnd_frame, 0, 0);
	TranslateMessage(&Msg);
	DispatchMessageA(&Msg);
	if (Msg.message == 18)
	{
		return_value = Msg.wParam;
		return 0;
	}
	return 1;
}

void winmain::memalloc_failure()
{
	/*midi_music_stop();
	Sound_Close();*/
	gdrv::uninit();
	char* caption = pinball::get_rc_string(170, 0);
	char* text = pinball::get_rc_string(179, 0);
	MessageBoxA(nullptr, text, caption, 0x2030u);
	_exit(1);
}

int winmain::check_expiration_date()
{
	return 0;
}


HDC winmain::_BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint)
{
	HDC context = BeginPaint(hWnd, lpPaint);
	if (hWnd && GetLayout(context) & 1)
		SetLayout(context, 0);
	return context;
}

HDC winmain::_GetDC(HWND hWnd)
{
	HDC context = GetDC(hWnd);
	if (hWnd && GetLayout(context) & 1)
		SetLayout(context, 0);
	return context;
}

int winmain::a_dialog(HINSTANCE hInstance, HWND hWnd)
{
	char appName[100];
	char szOtherStuff[100];

	lstrcpyA(appName, pinball::get_rc_string(38, 0));
	lstrcpyA(szOtherStuff, pinball::get_rc_string(102, 0));
	auto icon = LoadIconA(hInstance, "ICON_1");
	return ShellAboutA(hWnd, appName, szOtherStuff, icon);
}
