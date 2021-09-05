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

const double TargetFps = 60, TargetFrameTime = 1000 / TargetFps;

HCURSOR winmain::mouse_hsave;
SDL_Window* winmain::MainWindow = nullptr;
SDL_Renderer* winmain::Renderer = nullptr;
ImGuiIO* winmain::ImIO = nullptr;

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
bool winmain::ShowAboutDialog = false;
bool winmain::ShowImGuiDemo = false;


uint32_t timeGetTimeAlt()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return static_cast<uint32_t>(millis);
}

int winmain::WinMain(LPCSTR lpCmdLine)
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
	
	pinball::FindShiftKeys();
	options::init_resolution();

	// SDL window
	SDL_Window* window = SDL_CreateWindow
	(
		pinball::get_rc_string(38, 0),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		800, 556,
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
	Renderer = renderer;
	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create renderer", SDL_GetError(), window);
		return 1;
	}

	// ImGui init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, 0, 0);
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	ImIO = &io;
	// ImGui_ImplSDL2_Init is private, we are not actually using ImGui OpenGl backend
	ImGui_ImplSDL2_InitForOpenGL(window, nullptr);

	// PB init from message handler
	{
		++memory::critical_allocation;

		options::init(nullptr);
		auto voiceCount = options::get_int(nullptr, "Voices", 8);
		if (!Sound::Init(voiceCount))
			options::menu_set(Menu1_Sounds, 0);
		Sound::Activate();

		if (!pinball::quickFlag && !midi::music_init())
			options::menu_set(Menu1_Music, 0);

		if (pb::init()) 
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not load game data", "The .dat file is missing", window);
			return 1;
		}
		
		fullscrn::init();

		--memory::critical_allocation;
	}

	pb::reset_table();
	pb::firsttime_setup();

	if (strstr(lpCmdLine, "-fullscreen"))
	{
		options::Options.FullScreen = 1;
		options::menu_check(Menu1_Full_Screen, 1);
	}

	SDL_ShowWindow(window);
	fullscrn::set_screen_mode(options::Options.FullScreen);	

	pinball::adjust_priority(options::Options.PriorityAdj);

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
							auto plt = static_cast<ColorRgba*>(malloc(1024u));
							auto pltPtr = &plt[10];
							for (int i1 = 0, i2 = 0; i1 < 256 - 10; ++i1, i2 += 8)
							{
								unsigned char blue = i2, redGreen = i2;
								if (i2 > 255)
								{
									blue = 255;
									redGreen = i1;
								}

								auto clr = Rgba{redGreen, redGreen, blue, 0};
								*pltPtr++ = {*reinterpret_cast<uint32_t*>(&clr)};
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
				}
			}

			auto frameEnd = static_cast<double>(SDL_GetPerformanceCounter());
			auto elapsedMs = (frameEnd - frameStart) * sdlTimerResMs;
			if (elapsedMs >= TargetFrameTime)
			{
				// Keep track of remainder, limited to one frame time.
				frameStart = frameEnd - min(elapsedMs - TargetFrameTime, TargetFrameTime) / sdlTimerResMs;

				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();

				RenderUi();

				SDL_RenderClear(renderer);
				gdrv::BlitScreen();

				ImGui::Render();
				ImGuiSDL::Render(ImGui::GetDrawData());

				SDL_RenderPresent(renderer);
				frameCounter++;
			}
		}
	}

	gdrv::destroy_bitmap(&gfr_display);
	options::uninit();
	midi::music_shutdown();
	pb::uninit();
	Sound::Close();
	gdrv::uninit();
	ImGuiSDL::Deinitialize();
	ImGui_ImplSDL2_Shutdown();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	ImGui::DestroyContext();
	SDL_Quit();
	options::path_uninit();

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

void winmain::RenderUi()
{
	// No demo window in release to save space
#ifndef NDEBUG
	if (ShowImGuiDemo)
		ImGui::ShowDemoWindow();
#endif

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Game"))
		{
			if (ImGui::MenuItem("New Game", "F2"))
			{
				new_game();
			}
			if (ImGui::MenuItem("Launch Ball"))
			{
				end_pause();
				pb::launch_ball();
			}
			if (ImGui::MenuItem("Pause/ Resume Game", "F3"))
			{
				pause();
			}
			ImGui::Separator();

			if (ImGui::MenuItem("High Scores..."))
			{
				if (!single_step)
					pause();
				pb::high_scores();
			}
			if (ImGui::MenuItem("Demo"))
			{
				end_pause();
				pb::toggle_demo();
			}
			if (ImGui::MenuItem("Exit"))
			{
				SDL_Event event{SDL_QUIT};
				SDL_PushEvent(&event);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Full Screen", "F4", options::Options.FullScreen))
			{
				options::toggle(Menu1_Full_Screen);
			}
			if (ImGui::BeginMenu("Select Players"))
			{
				if (ImGui::MenuItem("1 Player", nullptr, options::Options.Players == 1))
				{
					options::toggle(Menu1_1Player);
					new_game();
				}
				if (ImGui::MenuItem("2 Players", nullptr, options::Options.Players == 2))
				{
					options::toggle(Menu1_2Players);
					new_game();
				}
				if (ImGui::MenuItem("3 Players", nullptr, options::Options.Players == 3))
				{
					options::toggle(Menu1_3Players);
					new_game();
				}
				if (ImGui::MenuItem("4 Players", nullptr, options::Options.Players == 4))
				{
					options::toggle(Menu1_4Players);
					new_game();
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Sound", nullptr, options::Options.Sounds))
			{
				options::toggle(Menu1_Sounds);
			}
			if (ImGui::MenuItem("Music", nullptr, options::Options.Music))
			{
				options::toggle(Menu1_Music);
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Player Controls...", "F8"))
			{
				if (!single_step)
					pause();
				options::keyboard();
			}
			if (ImGui::BeginMenu("Table Resolution"))
			{
				if (ImGui::MenuItem("Not implemented"))
				{
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Uniform Scaling", nullptr, options::Options.UniformScaling))
				{
					options::toggle(Menu1_WindowUniformScale);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
#ifndef NDEBUG
			if (ImGui::MenuItem("ImGui Demo"))
			{
				ShowImGuiDemo ^= true;
			}
#endif
			if (ImGui::MenuItem("Help Topics", "F1"))
			{
				if (!single_step)
					pause();
				help_introduction(nullptr, (HWND)MainWindow);
			}
			ImGui::Separator();

			if (ImGui::MenuItem("About Pinball"))
			{
				if (!single_step)
					pause();
				ShowAboutDialog = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	a_dialog();
	high_score::RenderHighScoreDialog();
}

int winmain::event_handler(const SDL_Event* event)
{
	ImGui_ImplSDL2_ProcessEvent(event);

	if (ImIO->WantCaptureMouse)
	{
		if (mouse_down)
		{
			mouse_down = 0;
			SDL_ShowCursor(SDL_ENABLE);
			SDL_SetWindowGrab(MainWindow, SDL_FALSE);
		}
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:
			return 1;
		default: ;
		}
	}
	if (ImIO->WantCaptureKeyboard)
	{
		switch (event->type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			return 1;
		default: ;
		}
	}

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
			help_introduction(nullptr, (HWND)MainWindow);
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
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED:
			fullscrn::window_size_changed();
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

void winmain::a_dialog()
{
	if (ShowAboutDialog == true)
	{
		ShowAboutDialog = false;
		ImGui::OpenPopup("About");
	}

	bool unused_open = true;
	if (ImGui::BeginPopupModal("About", &unused_open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("3D Pinball for Windows - Space Cadet");
		ImGui::TextUnformatted("Decompiled -> Ported to SDL");
		ImGui::Separator();

		if (ImGui::Button("Ok"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void winmain::end_pause()
{
	if (single_step)
	{
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
	SDL_Event event{SDL_QUIT};
	SDL_PushEvent(&event);
}
