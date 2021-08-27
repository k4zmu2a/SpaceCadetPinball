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

const double TargetFps = 60, TargetFrameTime = 1000 / TargetFps;

HINSTANCE winmain::hinst = nullptr;
HWND winmain::hwnd_frame = nullptr;
HCURSOR winmain::mouse_hsave;
SDL_Window* winmain::MainWindow = nullptr;

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

DWORD winmain::then;
DWORD winmain::now;
UINT winmain::iFrostUniqueMsg;
bool winmain::restart = false;

gdrv_bitmap8 winmain::gfr_display{};
char winmain::DatFileName[300]{};


uint32_t timeGetTimeAlt()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();	
	return static_cast<uint32_t>(millis);
}


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
	--memory::critical_allocation;

	// SDL init
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not initialize SDL2", SDL_GetError(), nullptr);
		return 1;
	}	

	pinball::quickFlag = strstr(lpCmdLine, "-quick") != nullptr;
	hinst = hInstance;
	auto regSpaceCadet = pinball::get_rc_string(166, 0);
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
	auto windowClass = pinball::get_rc_string(167, 0);
	auto windowHandle = FindWindowA(windowClass, nullptr);
	if (windowHandle)
	{
		SendMessageA(windowHandle, iFrostUniqueMsg, 0, 0);
		return 0;
	}

	if (check_expiration_date())
		return 0;

	auto splash = splash::splash_screen(hInstance, "splash_bitmap", "splash_bitmap");
	pinball::FindShiftKeys();
	options::init_resolution();

	// SDL window
	SDL_Window* window = SDL_CreateWindow
	(
		pinball::get_rc_string(38, 0),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE
	);
	MainWindow = window;
	if (!window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", SDL_GetError(), nullptr);
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer
	(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
	);
	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create renderer", SDL_GetError(), window);
		return 1;
	}

	// PB init from message handler
	{
		RECT rect{};
		++memory::critical_allocation;

		auto prevCursor = SetCursor(LoadCursorA(nullptr, IDC_WAIT));

		auto voiceCount = options::get_int(nullptr, "Voices", 8);
		if (!Sound::Init(voiceCount))
			options::menu_set(Menu1_Sounds, 0);
		Sound::Activate();

		if (!pinball::quickFlag && !midi::music_init((HWND)window))
			options::menu_set(Menu1_Music, 0);

		if (pb::init(renderer))
			_exit(0);
		SetCursor(prevCursor);
		auto changeDisplayFg = options::get_int(nullptr, "Change Display", 1);
		auto menuHandle = GetMenu((HWND)window);

		GetWindowRect(GetDesktopWindow(), &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		pb::window_size(&width, &height);
		fullscrn::init(width, height, options::Options.FullScreen, (HWND)window, menuHandle,
		               changeDisplayFg);

		--memory::critical_allocation;
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

	SDL_ShowWindow(window);
	fullscrn::set_screen_mode(options::Options.FullScreen);

	if (splash)
	{
		splash::splash_hide(splash);
		splash::splash_destroy(splash);
	}

	pinball::adjust_priority(options::Options.PriorityAdj);
	const auto startTime = timeGetTimeAlt();
	MSG wndMessage{};
	while (timeGetTimeAlt() >= startTime && timeGetTimeAlt() - startTime < 0) // Don't wait for now, was 2000
		PeekMessageA(&wndMessage, hwnd_frame, 0, 0, 1u);

	if (strstr(lpCmdLine, "-demo"))
		pb::toggle_demo();
	else
		pb::replay_level(0);

	DWORD updateCounter = 300u, frameCounter = 0, prevTime = 0u;
	then = timeGetTimeAlt();

	double sdlTimerResMs = 1000.0 / static_cast<double>(SDL_GetPerformanceFrequency());	
	auto frameStart = static_cast<double>(SDL_GetPerformanceCounter());
	while (true)
	{		
		if (!updateCounter)
		{
			updateCounter = 300;
			if (DispFrameRate)
			{
				auto curTime = timeGetTimeAlt();
				if (prevTime)
				{
					char buf[60];
					auto elapsedSec = static_cast<float>(curTime - prevTime) * 0.001f;
					sprintf_s(buf, "Updates/sec = %02.02f Frames/sec = %02.02f ",
						300.0f / elapsedSec, frameCounter / elapsedSec);
					SDL_SetWindowTitle(window, buf);
					frameCounter = 0;

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
		
		if (!ProcessWindowMessages() || bQuit)
			break;

		if (has_focus)
		{
			if (mouse_down)
			{
				now = timeGetTimeAlt();
				if (now - then >= 2)
				{
					int x, y;
					SDL_GetMouseState(&x, &y);
					pb::ballset(last_mouse_x - x, y - last_mouse_y);
					SDL_WarpMouseInWindow(window, last_mouse_x, last_mouse_y);
				}
			}
			if (!single_step)
			{
				auto curTime = timeGetTimeAlt();
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
						gdrv::fill_bitmap(&gfr_display, 1, 10, 299 - updateCounter, 0, fillChar);
					}
					--updateCounter;
					then = now;

					auto frameEnd =  static_cast<double>(SDL_GetPerformanceCounter());
					auto elapsedMs = (frameEnd - frameStart) * sdlTimerResMs;
					if (elapsedMs >= TargetFrameTime)
					{
						// Keep track of remainder, limited to one frame time.
						frameStart = frameEnd - min(elapsedMs - TargetFrameTime, TargetFrameTime) / sdlTimerResMs;
						SDL_RenderPresent(renderer);
						frameCounter++;												
					}
				}
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
	UnregisterClassA(windowClass, hinst);

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

			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case WM_KILLFOCUS:
			has_focus = 0;
			gdrv::get_focus();
			pb::loose_focus();
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case WM_CREATE:
			{
				RECT rect{};
				++memory::critical_allocation;

				auto prevCursor = SetCursor(LoadCursorA(nullptr, IDC_WAIT));
				gdrv::init(nullptr,0,0);

				auto voiceCount = options::get_int(nullptr, "Voices", 8);
				if (!Sound::Init(voiceCount))
					options::menu_set(Menu1_Sounds, 0);
				Sound::Activate();

				if (!pinball::quickFlag && !midi::music_init(hWnd))
					options::menu_set(Menu1_Music, 0);

				if (pb::init(nullptr))
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
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			}
		case WM_MOVE:
			no_time_loss = 1;
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case WM_SETFOCUS:
			has_focus = 1;
			no_time_loss = 1;
			gdrv::get_focus();
			fullscrn::force_redraw();
			pb::paint();
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
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
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case WM_ERASEBKGND:
			break;
		case WM_SIZE:
			fullscrn::window_size_changed();
			fullscrn::force_redraw();
			pb::paint();
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		default:
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		}
		return 0;
	}

	switch (Msg)
	{
	case WM_MENUSELECT:
		if (lParam)
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		if (fullscrn::screen_mode)
			fullscrn::set_menu_mode(0);
		return 0;
	case WM_SYSKEYDOWN:
		no_time_loss = 1;
		if (fullscrn::screen_mode)
			fullscrn::set_menu_mode(1);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_GETMINMAXINFO:
		fullscrn::getminmaxinfo((MINMAXINFO*)lParam);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_DISPLAYCHANGE:
		options::update_resolution_menu();
		if (fullscrn::displaychange())
		{
			options::Options.FullScreen = 0;
			options::menu_check(Menu1_Full_Screen, 0);
		}
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_KEYUP:
		pb::keyup(wParamI);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
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
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		switch (wParam)
		{
		case 'H':
			DispGRhistory = 1;
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case 'Y':
			SetWindowTextA(hWnd, "Pinball");
			DispFrameRate = DispFrameRate == 0;
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case VK_F1:
			pb::frame(10);
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case VK_F15:
			single_step = single_step == 0;
			if (single_step == 0)
				no_time_loss = 1;
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		default:
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
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
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case SC_SCREENSAVE:
			fullscrn::activate(0);
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		default: break;
		}
		end_pause();
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_INITMENU:
		no_time_loss = 1;
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
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
						auto caption = pinball::get_rc_string(170, 0);
						auto text = pinball::get_rc_string(171, 0);
						MessageBoxA(hwnd_frame, text, caption, 0x2010u);
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
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
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

			return DefWindowProcA(hWnd, Msg, wParam, lParam);
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
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_RBUTTONDOWN:
		if (!pb::cheat_mode)
			pb::keydown(options::Options.RightFlipperKey);
		if (pb::game_mode)
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		break;
	case WM_RBUTTONUP:
		if (!pb::cheat_mode)
			pb::keyup(options::Options.RightFlipperKey);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_MBUTTONDOWN:
		pb::keydown(options::Options.PlungerKey);
		if (pb::game_mode)
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		break;
	case WM_MBUTTONUP:
		pb::keyup(options::Options.PlungerKey);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_POWERBROADCAST:
		if (wParam == 4 && options::Options.FullScreen)
		{
			options::Options.FullScreen = 0;
			options::menu_check(Menu1_Full_Screen, 0);
			fullscrn::set_screen_mode(options::Options.FullScreen);
		}
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case WM_PALETTECHANGED:
		InvalidateRect(hWnd, nullptr, 0);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	case MM_MCINOTIFY:
		if (wParam == 1)
			midi::restart_midi_seq(lParam);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	default:
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}

	pb::mode_countdown(-1);
	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

int winmain::event_handler(const SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_QUIT:
		end_pause();
		bQuit = 1;
		PostQuitMessage(0);
		fullscrn::shutdown();
		return_value = 0;
		return 0;
	case SDL_KEYUP:
		pb::keyup(event->key.keysym.sym);
		break;
	case SDL_KEYDOWN:
		if (!event->key.repeat)
			pb::keydown(event->key.keysym.sym);
		switch (event->key.keysym.sym)
		{
		case SDLK_ESCAPE:
			if (options::Options.FullScreen)
				options::toggle(Menu1_Full_Screen);
			SDL_MinimizeWindow(MainWindow);
			break;
		case SDLK_F1:
			help_introduction(hinst, (HWND)MainWindow);
			break;
		case SDLK_F2:
			new_game();
			break;
		case SDLK_F3:
			pause();
			break;
		case SDLK_F4:
			options::toggle(Menu1_Full_Screen);
			break;
		case SDLK_F5:
			options::toggle(Menu1_Sounds);
			break;
		case SDLK_F6:
			options::toggle(Menu1_Music);
			break;
		case SDLK_F8:
			if (!single_step)
				pause();
			options::keyboard();
			break;
		default:
			break;
		}

		if (!pb::cheat_mode)
			break;

		switch (event->key.keysym.sym)
		{
		case SDLK_h:
			DispGRhistory = 1;
			break;
		case SDLK_y:
			SDL_SetWindowTitle(MainWindow, "Pinball");
			DispFrameRate = DispFrameRate == 0;
			break;
		case SDLK_F1:
			pb::frame(10);
			break;
		case SDLK_F10:
			single_step = single_step == 0;
			if (single_step == 0)
				no_time_loss = 1;
			break;
		default:
			break;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		switch (event->button.button)
		{
		case SDL_BUTTON_LEFT:
			if (pb::cheat_mode)
			{
				mouse_down = 1;
				last_mouse_x = event->button.x;
				last_mouse_y = event->button.y;
				SDL_ShowCursor(SDL_DISABLE);
				SDL_SetWindowGrab(MainWindow, SDL_TRUE);
			}
			else
				pb::keydown(options::Options.LeftFlipperKey);
			break;
		case SDL_BUTTON_RIGHT:
			if (!pb::cheat_mode)
				pb::keydown(options::Options.RightFlipperKey);
			break;
		case SDL_BUTTON_MIDDLE:
			pb::keydown(options::Options.PlungerKey);
			break;
		default:
			break;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		switch (event->button.button)
		{
		case SDL_BUTTON_LEFT:
			if (mouse_down)
			{
				mouse_down = 0;				
				SDL_ShowCursor(SDL_ENABLE);
				SDL_SetWindowGrab(MainWindow, SDL_FALSE);
			}
			if (!pb::cheat_mode)
				pb::keyup(options::Options.LeftFlipperKey);
			break;
		case SDL_BUTTON_RIGHT:
			if (!pb::cheat_mode)
				pb::keyup(options::Options.RightFlipperKey);
			break;
		case SDL_BUTTON_MIDDLE:
			pb::keyup(options::Options.PlungerKey);
			break;
		default:
			break;
		}
		break;
	case SDL_WINDOWEVENT:
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_TAKE_FOCUS:
		case SDL_WINDOWEVENT_EXPOSED:
		case SDL_WINDOWEVENT_SHOWN:
			activated = 1;
			Sound::Activate();
			if (options::Options.Music && !single_step)
				midi::play_pb_theme(0);
			no_time_loss = 1;
			pinball::adjust_priority(options::Options.PriorityAdj);
			has_focus = 1;
			gdrv::get_focus();
			fullscrn::force_redraw();
			pb::paint();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_HIDDEN:
			activated = 0;
			fullscrn::activate(0);
			options::menu_check(Menu1_Full_Screen, 0);
			options::Options.FullScreen = 0;
			SetThreadPriority(GetCurrentThread(), 0);
			Sound::Deactivate();
			midi::music_stop();
			has_focus = 0;
			gdrv::get_focus();
			pb::loose_focus();
			break;
		default: ;
		}
		break;
	default: ;
	}

	return 1;
}

int winmain::ProcessWindowMessages()
{
	SDL_Event event;
	if (has_focus && !single_step)
	{
		while (SDL_PollEvent(&event))
		{
			if (!event_handler(&event))
				return 0;
		}

		return 1;
	}

	SDL_WaitEvent(&event);
	return event_handler(&event);
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
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
			//HtmlHelpA(GetDesktopWindow(), buf2, 0, 0);
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
