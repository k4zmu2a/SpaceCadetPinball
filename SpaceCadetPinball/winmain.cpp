#include "pch.h"
#include "winmain.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pinball.h"
#include "options.h"
#include "pb.h"
#include "Sound.h"
#include "resource.h"
#include "splash.h"
#include "render.h"

const int TargetFrameTime = 8;

HINSTANCE winmain::hinst = nullptr;
HWND winmain::hwnd_frame = nullptr;
HCURSOR winmain::mouse_hsave;

int winmain::return_value = 0;
int winmain::bQuit = 0;
int winmain::activated;
int winmain::DispFrameRate = 0;
int winmain::DispGRhistory = 0;
int winmain::single_step = 0;
int winmain::has_focus = 1;
int winmain::last_mouse_x;
int winmain::last_mouse_y;
int winmain::mouse_down;
int winmain::no_time_loss;

UINT winmain::iFrostUniqueMsg;
bool winmain::restart = false;

gdrv_bitmap8 winmain::gfr_display{};
char winmain::DatFileName[300]{};


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
	options::ReadOptions();
	auto regSpaceCadet = pinball::get_rc_string(166, 0);

	// Needed for sub 16ms sleep
	if (timeBeginPeriod(1) == TIMERR_NOERROR)
		atexit(ResetTimer);

	if (options::get_int(regSpaceCadet, "Table Version", 1) <= 1)
	{
		auto tmpBuf = memory::allocate(0x1F4u);
		if (!tmpBuf)
		{
			options::path_uninit();
			return 0;
		}

		options::set_int(regSpaceCadet, "Table Version", 1u);
		GetModuleFileNameA(hinst, tmpBuf, 0x1F4u);
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
				bool setOption = false;
				for (int i = 0; i < 32700; ++i)
				{
					sprintf_s(Buffer, "Table%d", i);
					options::get_string(nullptr, Buffer, tmpBuf, "", 500);
					if (!*tmpBuf)
						break;
					options::get_string(tmpBuf, "Table Name", tmpBuf2, "", 500);
					if (!lstrcmpA(tmpBuf2, pinball::get_rc_string(169, 0)))
					{
						setOption = false;
						break;
					}
					if (!*tmpBuf2)
						break;
				}
				if (setOption)
					options::set_string(nullptr, Buffer, regSpaceCadet);
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
		options::get_string(regSpaceCadet, "Shell Exe", tmpBuf, "", 500);
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
	hinst = hInstance;
	options::get_string(regSpaceCadet, "Pinball Data", DatFileName, pinball::get_rc_string(168, 0), 300);

	/*Check for full tilt .dat file and switch to it automatically*/
	char cadetFilePath[300]{};
	pinball::make_path_name(cadetFilePath, "CADET.DAT", 300);
	FILE* cadetDat;
	fopen_s(&cadetDat, cadetFilePath, "r");
	if (cadetDat)
	{
		fclose(cadetDat);
		strcpy_s(DatFileName, "CADET.DAT");
		pb::FullTiltMode = true;
	}

	iFrostUniqueMsg = RegisterWindowMessageA("PinballThemeSwitcherUniqueMsgString");
	auto windowClass = pinball::get_rc_Wstring(167, 0);
	auto windowHandle = FindWindowW(windowClass, nullptr);
	if (windowHandle)
	{
		SendMessageW(windowHandle, iFrostUniqueMsg, 0, 0);
		return 0;
	}

	if (check_expiration_date())
		return 0;

	INITCOMMONCONTROLSEX picce;
	picce.dwSize = 8;
	picce.dwICC = 5885;
	InitCommonControlsEx(&picce);

	WNDCLASSEXW wndClass{};
	wndClass.cbSize = sizeof wndClass;
	wndClass.style = CS_DBLCLKS | CS_BYTEALIGNCLIENT;
	wndClass.lpfnWndProc = message_handler;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIconW(hInstance, L"ICON_1");
	wndClass.hCursor = LoadCursorW(nullptr, (PWSTR)IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)16;
	wndClass.lpszMenuName = L"MENU_1";
	wndClass.lpszClassName = windowClass;
	auto splash = splash::splash_screen(hInstance, "splash_bitmap", "splash_bitmap");
	RegisterClassExW(&wndClass);

	pinball::FindShiftKeys();
	options::init_resolution();

	// Some languages use RTL layout
	DWORD dwExStyle;
	switch (options::Options.Language)
	{
	case Languages::Hebrew:
	case Languages::Arabic:
		dwExStyle = WS_EX_LAYOUTRTL;
		break;
	default:
		dwExStyle = 0;
	}
	auto windowName = pinball::get_rc_Wstring(38, 0);
	windowHandle = CreateWindowExW(dwExStyle, windowClass, windowName,
		WndStyle, 0, 0, 640, 480, nullptr, nullptr, hInstance, nullptr);

	hwnd_frame = windowHandle;
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
		options::menu_check(Menu1_Full_Screen, 1);
	}

	ShowWindow(hwnd_frame, nShowCmd);
	fullscrn::set_screen_mode(options::Options.FullScreen);
	UpdateWindow(hwnd_frame);

	if (splash)
	{
		splash::splash_hide(splash);
		splash::splash_destroy(splash);
	}

	pinball::adjust_priority(options::Options.PriorityAdj);
	const auto startTime = timeGetTime();
	MSG wndMessage{};
	while (timeGetTime() >= startTime && timeGetTime() - startTime < 0) // Don't wait for now, was 2000
		PeekMessageA(&wndMessage, hwnd_frame, 0, 0, 1u);

	if (strstr(lpCmdLine, "-demo"))
		pb::toggle_demo();
	else
		pb::replay_level(0);

	DWORD someTimeCounter = 300u, prevTime = 0u;
	int sleepRemainder = 0, frameDuration = TargetFrameTime;
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
					auto dt = static_cast<float>(curTime - prevTime) * 0.001f;
					if (!options::Options.AlternativeRender)
						sprintf_s(buf, "Frames/sec = %02.02f", 300.0f / dt);
					else
					{
						sprintf_s(buf, "Updates/sec = %02.02f Frames/sec = %02.02f",
						          300.0f / dt, pb::frameCounter / dt);
						pb::frameCounter = 0;
					}

					SetWindowTextA(hwnd_frame, buf);
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
			if (mouse_down && frameDuration >= 2)
			{
				/*last_mouse_n is in client coordinates*/
				POINT Point;
				GetCursorPos(&Point);
				ScreenToClient(hwnd_frame, &Point);
				pb::ballset(last_mouse_x - Point.x, Point.y - last_mouse_y);
				Point = POINT{ last_mouse_x, last_mouse_y };
				ClientToScreen(hwnd_frame, &Point);
				SetCursorPos(Point.x, Point.y);
			}

			if (!single_step)
			{
				auto frameStart = timeGetTime();
				auto dt = frameDuration;
				if (!no_time_loss)
					pb::frame(dt);
				else
					no_time_loss = 0;

				if (DispGRhistory)
				{
					auto width = render::vscreen.Width / 2;
					auto height = 64;
					if (!gfr_display.BmpBufPtr1)
					{
						gdrv::create_bitmap(&gfr_display, width, height);
					}

					gdrv::ScrollBitmapHorizontal(&gfr_display, -1);

					float target = TargetFrameTime;
					auto scale = height / target / 2;
					gdrv::fill_bitmap(&gfr_display, 1, height, width - 1, 0, 0); // Background

					auto targetVal = dt < target ? dt : target;
					auto targetHeight = min(static_cast<int>(std::round(targetVal * scale)), height);
					gdrv::fill_bitmap(&gfr_display, 1, targetHeight, width - 1, height - targetHeight, -1); // Target

					auto diffVal = dt < target ? target - dt : dt - target;
					auto diffHeight = min(static_cast<int>(std::round(diffVal * scale)), height);
					gdrv::fill_bitmap(&gfr_display, 1, diffHeight, width - 1, height - targetHeight - diffHeight, 1); // Target diff

					gdrv::blit(&gfr_display, 0, 0, render::vscreen.Width - width, 0, width, height);
				}

				auto updateEnd = timeGetTime();
				auto sleepDuration = TargetFrameTime - (int)(updateEnd - frameStart) - sleepRemainder;

				if (sleepDuration > 0)
					Sleep(sleepDuration);

				auto frameEnd = timeGetTime();
				sleepRemainder = (frameEnd - updateEnd) - sleepDuration;
				frameDuration = min(frameEnd - frameStart, TargetFrameTime * 2);

				--someTimeCounter;
			}
		}
	}

	gdrv::destroy_bitmap(&gfr_display);
	options::uninit();
	midi::music_shutdown();
	pb::uninit();
	Sound::Close();
	gdrv::uninit();
	DestroyWindow(hwnd_frame);
	options::path_uninit();
	UnregisterClassW(windowClass, hinst);

	if (restart)
	{
		char restartPath[300]{};
		if (GetModuleFileNameA(nullptr, restartPath, 300))
		{
			STARTUPINFO si{};
			PROCESS_INFORMATION pi{};
			si.cb = sizeof si;
			if (CreateProcess(restartPath, nullptr, nullptr, nullptr,
			                  FALSE, 0, nullptr, nullptr, &si, &pi))
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
	}

	return return_value;
}

LRESULT CALLBACK winmain::message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int wParamI = static_cast<int>(wParam);

	if (Msg == iFrostUniqueMsg)
	{
		if (IsIconic(hWnd))
			ShowWindow(hWnd, 9);
		SetForegroundWindow(hWnd);
		return 0;
	}

	if (Msg <= WM_ACTIVATEAPP)
	{
		switch (Msg)
		{
		case WM_ACTIVATEAPP:
			if (wParam)
			{
				activated = 1;
				Sound::Activate();
				if (options::Options.Music && !single_step)
					midi::play_pb_theme(0);
				no_time_loss = 1;
				pinball::adjust_priority(options::Options.PriorityAdj);
			}
			else
			{
				activated = 0;
				fullscrn::activate(0);
				options::menu_check(Menu1_Full_Screen, 0);
				options::Options.FullScreen = 0;
				SetThreadPriority(GetCurrentThread(), 0);
				Sound::Deactivate();
				midi::music_stop();
			}

			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case WM_KILLFOCUS:
			has_focus = 0;
			gdrv::get_focus();
			pb::loose_focus();
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case WM_CREATE:
			{
				RECT rect{};
				++memory::critical_allocation;

				auto prevCursor = SetCursor(LoadCursorA(nullptr, IDC_WAIT));
				gdrv::init(hinst, hWnd);

				auto voiceCount = options::get_int(nullptr, "Voices", 8);
				if (!Sound::Init(hinst, voiceCount, nullptr))
					options::menu_set(Menu1_Sounds, 0);
				Sound::Activate();

				if (!pinball::quickFlag && !midi::music_init(hWnd))
					options::menu_set(Menu1_Music, 0);

				if (pb::init())
					_exit(0);
				SetCursor(prevCursor);
				auto changeDisplayFg = options::get_int(nullptr, "Change Display", 1);
				auto menuHandle = GetMenu(hWnd);

				GetWindowRect(GetDesktopWindow(), &rect);
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				pb::window_size(&width, &height);
				fullscrn::init(width, height, options::Options.FullScreen, hWnd, menuHandle,
				               changeDisplayFg);

				--memory::critical_allocation;
				return DefWindowProcW(hWnd, Msg, wParam, lParam);
			}
		case WM_MOVE:
			no_time_loss = 1;
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case WM_SETFOCUS:
			has_focus = 1;
			no_time_loss = 1;
			gdrv::get_focus();
			fullscrn::force_redraw();
			pb::paint();
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case WM_PAINT:
			{
				PAINTSTRUCT paint{};
				_BeginPaint(hWnd, &paint);
				fullscrn::paint();
				EndPaint(hWnd, &paint);
				break;
			}
		case WM_CLOSE:
		case WM_QUIT:
		case WM_DESTROY:
			end_pause();
			bQuit = 1;
			PostQuitMessage(0);
			fullscrn::shutdown();
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case WM_ERASEBKGND:
			break;
		case WM_SIZE:
			fullscrn::window_size_changed();
			fullscrn::force_redraw();
			pb::paint();
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		default:
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		}
		return 0;
	}

	switch (Msg)
	{
	case WM_MENUSELECT:
		if (lParam)
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		if (fullscrn::screen_mode)
			fullscrn::set_menu_mode(0);
		return 0;
	case WM_SYSKEYDOWN:
		no_time_loss = 1;
		if (fullscrn::screen_mode)
			fullscrn::set_menu_mode(1);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_GETMINMAXINFO:
		fullscrn::getminmaxinfo((MINMAXINFO*)lParam);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_DISPLAYCHANGE:
		options::update_resolution_menu();
		if (fullscrn::displaychange())
		{
			options::Options.FullScreen = 0;
			options::menu_check(Menu1_Full_Screen, 0);
		}
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_KEYUP:
		pb::keyup(wParamI);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_KEYDOWN:
		if (!(lParam & 0x40000000))
			pb::keydown(wParamI);
		switch (wParam)
		{
		case VK_ESCAPE:
			if (options::Options.FullScreen)
				options::toggle(0x193u);
			SendMessageA(hwnd_frame, 0x112u, 0xF020u, 0);
			break;
		case VK_F1:
			help_introduction(hinst, hWnd);
			break;
		case VK_F2:
			new_game();
			break;
		case VK_F3:
			pause();
			break;
		case VK_F4:
			options::toggle(0x193u);
			break;
		case VK_F8:
			if (!single_step)
				pause();
			options::keyboard();
			break;
		default:
			break;
		}
		if (!pb::cheat_mode)
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		switch (wParam)
		{
		case 'G':
			DispGRhistory ^= 1;
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case 'Y':
			SetWindowTextA(hWnd, "Pinball");
			DispFrameRate = DispFrameRate == 0;
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case 'O':
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

					*pltPtr++ = { redGreen, redGreen, blue };
				}
				gdrv::display_palette(plt);
				free(plt);
			}
			break;
		case VK_F1:
			pb::frame(10);
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case VK_F15:
			single_step = single_step == 0;
			if (single_step == 0)
				no_time_loss = 1;
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		default:
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		}
	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
		{
		case SC_MOVE:
			if (fullscrn::screen_mode)
				return 0;
			break;
		case SC_MINIMIZE:
			if (!single_step)
				pause();
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		case SC_SCREENSAVE:
			fullscrn::activate(0);
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		default: break;
		}
		end_pause();
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_INITMENU:
		no_time_loss = 1;
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_COMMAND:
		no_time_loss = 1;
		switch (wParam)
		{
		case Menu1_Launch_Ball:
			end_pause();
			pb::launch_ball();
			break;
		case Menu1_Pause_Resume_Game:
			pause();
			break;
		case Menu1_Demo:
			end_pause();
			pb::toggle_demo();
			break;
		case Menu1_Select_Table:
			{
				if (!single_step)
					pause();
				auto tmpBuf = memory::allocate(0x1F4u);
				if (tmpBuf)
				{
					char cmdLine[0x1F4u];
					options::get_string(nullptr, "Shell Exe", tmpBuf, "", 500);
					auto iHwnd = reinterpret_cast<size_t>(hwnd_frame);
					sprintf_s(
						cmdLine,
						"%s %s%zX  %s%zX",
						tmpBuf,
						"select=",
						iHwnd,
						"confirm=",
						iHwnd * iHwnd * iHwnd * iHwnd * iHwnd * iHwnd * iHwnd);
					if (static_cast<int>(WinExec(cmdLine, 5u)) < 32)
					{
						auto caption = pinball::get_rc_Wstring(170, 0);
						auto text = pinball::get_rc_Wstring(171, 0);
						MessageBoxW(hwnd_frame, text, caption, 0x2010u);
					}
					memory::free(tmpBuf);
				}
				break;
			}
		case Menu1_1Player:
		case Menu1_2Players:
		case Menu1_3Players:
		case Menu1_4Players:
			options::toggle(wParamI);
			new_game();
			break;
		case Menu1_MaximumResolution:
		case Menu1_640x480:
		case Menu1_800x600:
		case Menu1_1024x768:
		case Menu1_WindowUniformScale:
		case Menu1_AlternativeRender:
			options::toggle(wParamI);
			break;
		case Menu1_Help_Topics:
			if (!single_step)
				pause();
			help_introduction(hinst, hWnd);
			break;
		case 106: // End game button?
			pb::end_game();
			break;
		case Menu1_Full_Screen:
		case Menu1_Sounds:
		case Menu1_Music:
			if (!single_step)
				pause();
			options::toggle(wParamI);
			break;
		case Menu1_Player_Controls:
		case 204: // Second controls button?
			if (!single_step)
				pause();
			options::keyboard();
			break;
		case Menu1_Exit:
			PostMessageA(hWnd, WM_QUIT, 0, 0);
			break;
		case Menu1_New_Game:
			new_game();
			break;
		case Menu1_About_Pinball:
			if (!single_step)
				pause();
			a_dialog(hinst, hWnd);
			break;
		case Menu1_High_Scores:
			if (!single_step)
				pause();
			pb::high_scores();
			break;
		case 1: // Unknown button
			midi::restart_midi_seq(lParam);
			break;
		default:
			break;
		}

		if (wParam >= Menu1_Language && wParam < Menu1_LanguageMax)
			options::toggle(wParamI);

		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_LBUTTONDOWN:
		if (pb::game_mode)
		{
			if (pb::cheat_mode)
			{
				mouse_down = 1;
				mouse_hsave = SetCursor(nullptr);
				auto mouseXY = fullscrn::convert_mouse_pos(static_cast<unsigned>(lParam));
				last_mouse_x = mouseXY & 0xffFFu;
				last_mouse_y = mouseXY >> 16;
				SetCapture(hWnd);
			}
			else
				pb::keydown(options::Options.LeftFlipperKey);

			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		}
		break;
	case WM_LBUTTONUP:
		if (mouse_down)
		{
			mouse_down = 0;
			SetCursor(mouse_hsave);
			ReleaseCapture();
		}
		if (!pb::cheat_mode)
			pb::keyup(options::Options.LeftFlipperKey);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_RBUTTONDOWN:
		if (!pb::cheat_mode)
			pb::keydown(options::Options.RightFlipperKey);
		if (pb::game_mode)
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		break;
	case WM_RBUTTONUP:
		if (!pb::cheat_mode)
			pb::keyup(options::Options.RightFlipperKey);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_MBUTTONDOWN:
		pb::keydown(options::Options.PlungerKey);
		if (pb::game_mode)
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		break;
	case WM_MBUTTONUP:
		pb::keyup(options::Options.PlungerKey);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_POWERBROADCAST:
		if (wParam == 4 && options::Options.FullScreen)
		{
			options::Options.FullScreen = 0;
			options::menu_check(Menu1_Full_Screen, 0);
			fullscrn::set_screen_mode(options::Options.FullScreen);
		}
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case WM_PALETTECHANGED:
		InvalidateRect(hWnd, nullptr, 0);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	case MM_MCINOTIFY:
		if (wParam == 1)
			midi::restart_midi_seq(lParam);
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}

	pb::mode_countdown(-1);
	return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

int winmain::ProcessWindowMessages()
{
	MSG Msg{};

	if (has_focus && !single_step)
	{
		while (PeekMessageA(&Msg, nullptr, 0, 0, 1u))
		{
			TranslateMessage(&Msg);
			DispatchMessageA(&Msg);
			if (Msg.message == 18)
			{
				return_value = static_cast<int>(Msg.wParam);
				return 0;
			}
		}
		return 1;
	}
	GetMessageA(&Msg, hwnd_frame, 0, 0);
	TranslateMessage(&Msg);
	DispatchMessageA(&Msg);
	if (Msg.message == 18)
	{
		return_value = static_cast<int>(Msg.wParam);
		return 0;
	}
	return 1;
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
	gdrv::uninit();
	auto caption = pinball::get_rc_Wstring(170, 0);
	auto text = pinball::get_rc_Wstring(179, 0);
	MessageBoxW(nullptr, text, caption, 0x2030u);
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
	PCWSTR appName = pinball::get_rc_Wstring(38, 0);
	PWSTR szOtherStuff = pinball::get_rc_Wstring(102, 0);

	lstrcatW(szOtherStuff, L" Decompilation version 1.1.2");
	auto icon = LoadIconW(hInstance, L"ICON_1");
	return ShellAboutW(hWnd, appName, szOtherStuff, icon);
}

void winmain::end_pause()
{
	if (single_step)
	{
		if (fullscrn::screen_mode)
			fullscrn::set_menu_mode(0);
		pb::pause_continue();
		no_time_loss = 1;
	}
}

void winmain::new_game()
{
	end_pause();
	HCURSOR prevCursor = SetCursor(LoadCursorA(nullptr, IDC_WAIT));
	pb::replay_level(0);
	SetCursor(prevCursor);
}

void winmain::pause()
{
	if (fullscrn::screen_mode)
	{
		if (single_step)
			fullscrn::set_menu_mode(0);
		else
			fullscrn::set_menu_mode(1);
	}
	pb::pause_continue();
	no_time_loss = 1;
}

void winmain::help_introduction(HINSTANCE a1, HWND a2)
{
	char* buf1 = memory::allocate(0x1F4u);
	if (buf1)
	{
		char* buf2 = memory::allocate(0x1F4u);
		if (buf2)
		{
			options::get_string(nullptr, "HelpFile", buf1, pinball::get_rc_string(178, 0), 500);
			options::get_string(pinball::get_rc_string(166, 0), "HelpFile", buf1, buf1, 500);
			lstrcpyA(buf2, buf1);
			memory::free(buf1);
			HtmlHelpA(GetDesktopWindow(), buf2, 0, 0);
			memory::free(buf2);
		}
		else
		{
			memory::free(buf1);
		}
	}
}

void winmain::Restart()
{
	restart = true;
	PostMessageA(hwnd_frame, WM_QUIT, 0, 0);
}
