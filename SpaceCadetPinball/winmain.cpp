#include "pch.h"
#include "winmain.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pinball.h"
#include "options.h"
#include "pb.h"
#include "render.h"
#include "Sound.h"

#include <functional>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <unistd.h>
#endif

SDL_Window *winmain::MainWindow = nullptr;
SDL_Renderer *winmain::Renderer = nullptr;
ImGuiIO *winmain::ImIO = nullptr;

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

bool winmain::restart = false;

gdrv_bitmap8 winmain::gfr_display{};
std::string winmain::DatFileName;
bool winmain::ShowAboutDialog = false;
bool winmain::ShowImGuiDemo = false;
bool winmain::ShowSpriteViewer = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
char *winmain::BasePath;
std::string winmain::FpsDetails;
double winmain::UpdateToFrameRatio;
winmain::DurationMs winmain::TargetFrameTime;

static bool loop_stop = false;

void run_loop(std::function<void()> fn)
{
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg([](void *arg)
								 {
									 auto *fn_ptr = (std::function<void()> *)arg;
									 if (!loop_stop && fn_ptr != nullptr)
									 {
										 auto &fn = *fn_ptr;
										 fn();
									 }
								 },
								 (void *)&fn, 60, 1);
#else
	while (!loop_stop)
	{
		fn();
	}
#endif
}

int winmain::WinMain(LPCSTR lpCmdLine)
{
	restart = false;
	bQuit = false;

	memory::init(memalloc_failure);

	// SDL init
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not initialize SDL2", SDL_GetError(), nullptr);
		return 1;
	}
#ifndef __EMSCRIPTEN__
	BasePath = SDL_GetBasePath();
#else
	BasePath = strdup("/game_resources/");
	chdir(BasePath);
#endif

	pinball::quickFlag = strstr(lpCmdLine, "-quick") != nullptr;
	DatFileName = options::get_string("Pinball Data", pinball::get_rc_string(168, 0));

	/*Check for full tilt .dat file and switch to it automatically*/
	auto cadetFilePath = pinball::make_path_name("CADET.DAT");
	auto cadetDat = fopen(cadetFilePath.c_str(), "r");
	if (cadetDat)
	{
		fclose(cadetDat);
		DatFileName = "CADET.DAT";
		pb::FullTiltMode = true;
	}

	// SDL window
	SDL_Window *window = SDL_CreateWindow(
		pinball::get_rc_string(38, 0),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		600, 440,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
	MainWindow = window;
	if (!window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", SDL_GetError(), nullptr);
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED);
	Renderer = renderer;
	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create renderer", SDL_GetError(), window);
		return 1;
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	// ImGui init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, 0, 0);
	ImGui::StyleColorsDark();
	ImGuiIO &io = ImGui::GetIO();
	ImIO = &io;
	// ImGui_ImplSDL2_Init is private, we are not actually using ImGui OpenGl backend
	ImGui_ImplSDL2_InitForOpenGL(window, nullptr);

	auto prefPath = SDL_GetPrefPath(nullptr, "SpaceCadetPinball");
	auto iniPath = std::string(prefPath) + "imgui_pb.ini";
	io.IniFilename = iniPath.c_str();
	SDL_free(prefPath);

	// PB init from message handler
	{
		++memory::critical_allocation;

		options::init();
		auto voiceCount = options::get_int("Voices", 8);
		if (Sound::Init(voiceCount))
			options::Options.Sounds = 0;
		Sound::Activate();

		if (!pinball::quickFlag && !midi::music_init())
			options::Options.Music = 0;

		if (pb::init())
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not load game data",
									 "The .dat file is missing", window);
			return 1;
		}

		fullscrn::init();

		--memory::critical_allocation;
	}

	pb::reset_table();
	pb::firsttime_setup();

#ifndef __EMSCRIPTEN__
	if (strstr(lpCmdLine, "-fullscreen"))
	{
		options::Options.FullScreen = 1;
	}
#else
	options::Options.FullScreen = 0;
#endif

	SDL_ShowWindow(window);
	fullscrn::set_screen_mode(options::Options.FullScreen);

	if (strstr(lpCmdLine, "-demo"))
		pb::toggle_demo();
	else
		pb::replay_level(0);

	DWORD dtHistoryCounter = 300u, updateCounter = 0, frameCounter = 0;

	auto frameStart = Clock::now();
	double frameDuration = TargetFrameTime.count(), UpdateToFrameCounter = 0;
	DurationMs sleepRemainder(0);
	auto prevTime = frameStart;
	run_loop([&]() {
		if (DispFrameRate)
		{
			auto curTime = Clock::now();
			if (curTime - prevTime > DurationMs(1000))
			{
				char buf[60];
				auto elapsedSec = DurationMs(curTime - prevTime).count() * 0.001;
				snprintf(buf, sizeof buf, "Updates/sec = %02.02f Frames/sec = %02.02f ",
				         updateCounter / elapsedSec, frameCounter / elapsedSec);
				SDL_SetWindowTitle(window, buf);
				FpsDetails = buf;
				frameCounter = updateCounter = 0;
				prevTime = curTime;
			}
		}

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

					*pltPtr++ = ColorRgba{Rgba{redGreen, redGreen, blue, 0}};
				}
				gdrv::display_palette(plt);
				free(plt);
				gdrv::create_bitmap(&gfr_display, 400, 15, 400, false);
			}

			if (!dtHistoryCounter)
			{
				dtHistoryCounter = 300;
				gdrv::copy_bitmap(&render::vscreen, 300, 10, 0, 30, &gfr_display, 0, 0);
				gdrv::fill_bitmap(&gfr_display, 300, 10, 0, 0, 0);
			}
		}

		if (!ProcessWindowMessages() || bQuit) {
			loop_stop = true;
			return;
		}

		if (has_focus)
		{
			if (mouse_down)
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				pb::ballset(last_mouse_x - x, y - last_mouse_y);
				SDL_WarpMouseInWindow(window, last_mouse_x, last_mouse_y);
			}
			if (!single_step)
			{
				auto deltaT = static_cast<int>(frameDuration);
				frameDuration -= deltaT;
				pb::frame(deltaT);
				if (gfr_display.BmpBufPtr1)
				{
					auto deltaTPal = deltaT + 10;
					auto fillChar = static_cast<uint8_t>(deltaTPal);
					if (deltaTPal > 236)
					{
						fillChar = 1;
					}
					gdrv::fill_bitmap(&gfr_display, 1, 10, 300 - dtHistoryCounter, 0, fillChar);
					--dtHistoryCounter;
				}
				updateCounter++;
			}

			if (UpdateToFrameCounter >= UpdateToFrameRatio)
			{
				UpdateToFrameCounter -= UpdateToFrameRatio;
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();

				RenderUi();

				SDL_RenderClear(renderer);
				render::PresentVScreen();

				ImGui::Render();
				ImGuiSDL::Render(ImGui::GetDrawData());

				SDL_RenderPresent(renderer);
				frameCounter++;
			}

			auto sdlError = SDL_GetError();
			if (sdlError[0])
			{
				SDL_ClearError();
				printf("SDL Error: %s\n", sdlError);
			}

			auto updateEnd = Clock::now();
			auto targetTimeDelta = TargetFrameTime - DurationMs(updateEnd - frameStart) - sleepRemainder;

			TimePoint frameEnd;
			if (targetTimeDelta > DurationMs::zero())
			{
				std::this_thread::sleep_for(targetTimeDelta);
				frameEnd = Clock::now();
				sleepRemainder = DurationMs(frameEnd - updateEnd) - targetTimeDelta;
			}
			else
			{
				frameEnd = updateEnd;
				sleepRemainder = DurationMs(0);
			}

			// Limit duration to 2 * target time
			frameDuration = std::min(frameDuration + DurationMs(frameEnd - frameStart).count(),
			                         2 * TargetFrameTime.count());
			frameStart = frameEnd;
			UpdateToFrameCounter++;
		}
	});

	gdrv::destroy_bitmap(&gfr_display);
	options::uninit();
	midi::music_shutdown();
	pb::uninit();
	Sound::Close();
	ImGuiSDL::Deinitialize();
	ImGui_ImplSDL2_Shutdown();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	ImGui::DestroyContext();
	SDL_Quit();

	return return_value;
}

void winmain::RenderUi()
{
	// No demo window in release to save space
#ifndef NDEBUG
	if (ShowImGuiDemo)
		ImGui::ShowDemoWindow(&ShowImGuiDemo);
#endif

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Game"))
		{
			if (ImGui::MenuItem("New Game", "F2"))
			{
				new_game();
			}
			if (ImGui::MenuItem("Launch Ball", nullptr, false, LaunchBallEnabled))
			{
				end_pause();
				pb::launch_ball();
			}
			if (ImGui::MenuItem("Pause/ Resume Game", "F3"))
			{
				pause();
			}
			ImGui::Separator();

			if (ImGui::MenuItem("High Scores...", nullptr, false, HighScoresEnabled))
			{
				if (!single_step)
					pause();
				pb::high_scores();
			}
			if (ImGui::MenuItem("Demo", nullptr, DemoActive))
			{
				end_pause();
				pb::toggle_demo();
			}
#ifndef __EMSCRIPTEN__
			if (ImGui::MenuItem("Exit"))
			{
				SDL_Event event{SDL_QUIT};
				SDL_PushEvent(&event);
			}
#endif
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
#ifndef __EMSCRIPTEN__
			if (ImGui::MenuItem("Full Screen", "F4", options::Options.FullScreen))
			{
				options::toggle(Menu1::Full_Screen);
			}
#endif
			if (ImGui::BeginMenu("Select Players"))
			{
				if (ImGui::MenuItem("1 Player", nullptr, options::Options.Players == 1))
				{
					options::toggle(Menu1::OnePlayer);
					new_game();
				}
				if (ImGui::MenuItem("2 Players", nullptr, options::Options.Players == 2))
				{
					options::toggle(Menu1::TwoPlayers);
					new_game();
				}
				if (ImGui::MenuItem("3 Players", nullptr, options::Options.Players == 3))
				{
					options::toggle(Menu1::ThreePlayers);
					new_game();
				}
				if (ImGui::MenuItem("4 Players", nullptr, options::Options.Players == 4))
				{
					options::toggle(Menu1::FourPlayers);
					new_game();
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Sound", nullptr, options::Options.Sounds))
			{
				options::toggle(Menu1::Sounds);
			}
			if (ImGui::MenuItem("Music", nullptr, options::Options.Music))
			{
				options::toggle(Menu1::Music);
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
				char buffer[20]{};
				auto maxResText = pinball::get_rc_string(fullscrn::GetMaxResolution() + 2030, 0);
				if (ImGui::MenuItem(maxResText, nullptr, options::Options.Resolution == -1))
				{
					options::toggle(Menu1::MaximumResolution);
				}
				for (auto i = 0; i <= fullscrn::GetMaxResolution(); i++)
				{
					auto &res = fullscrn::resolution_array[i];
					snprintf(buffer, sizeof buffer - 1, "%d x %d", res.ScreenWidth, res.ScreenHeight);
					if (ImGui::MenuItem(buffer, nullptr, options::Options.Resolution == i))
					{
						options::toggle(static_cast<Menu1>(static_cast<int>(Menu1::R640x480) + i));
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Graphics"))
			{
				if (ImGui::MenuItem("Uniform Scaling", nullptr, options::Options.UniformScaling))
				{
					options::toggle(Menu1::WindowUniformScale);
				}
				if (ImGui::MenuItem("Linear Filtering", nullptr, options::Options.LinearFiltering))
				{
					options::toggle(Menu1::WindowLinearFilter);
				}
				ImGui::DragFloat("UI Scale", &ImIO->FontGlobalScale, 0.005f, 0.8f, 5,
				                 "%.2f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Separator();

				auto changed = false;
				if (ImGui::MenuItem("Set Default UPS/FPS"))
				{
					changed = true;
					options::Options.UpdatesPerSecond = options::DefUps;
					options::Options.FramesPerSecond = options::DefFps;
				}
				if (ImGui::DragInt("UPS", &options::Options.UpdatesPerSecond, 1, options::MinUps, options::MaxUps,
				                   "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					options::Options.FramesPerSecond = std::min(options::Options.UpdatesPerSecond,
					                                            options::Options.FramesPerSecond);
				}
				if (ImGui::DragInt("FPS", &options::Options.FramesPerSecond, 1, options::MinFps, options::MaxFps,
				                   "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					options::Options.UpdatesPerSecond = std::max(options::Options.UpdatesPerSecond,
					                                             options::Options.FramesPerSecond);
				}
				if (changed)
				{
					UpdateFrameRate();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
#ifndef NDEBUG
			if (ImGui::MenuItem("ImGui Demo", nullptr, ShowImGuiDemo))
			{
				ShowImGuiDemo ^= true;
			}

			if (ImGui::MenuItem("Sprite Viewer", nullptr, ShowSpriteViewer))
			{
				if (!ShowSpriteViewer && !single_step)
					pause();
				ShowSpriteViewer ^= true;
			}
			ImGui::Separator();
#endif

			if (ImGui::MenuItem("About Pinball"))
			{
				if (!single_step)
					pause();
				ShowAboutDialog = true;
			}
			ImGui::EndMenu();
		}
		if (DispFrameRate && !FpsDetails.empty())
			if (ImGui::BeginMenu(FpsDetails.c_str()))
				ImGui::EndMenu();
		ImGui::EndMainMenuBar();
	}

	a_dialog();
	high_score::RenderHighScoreDialog();
	if (ShowSpriteViewer)
		render::SpriteViewer(&ShowSpriteViewer);
}

int winmain::event_handler(const SDL_Event *event)
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
		default:;
		}
	}
	if (ImIO->WantCaptureKeyboard)
	{
		switch (event->type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			return 1;
		default:;
		}
	}

	switch (event->type)
	{
	case SDL_QUIT:
		end_pause();
		bQuit = 1;
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
				options::toggle(Menu1::Full_Screen);
			SDL_MinimizeWindow(MainWindow);
			break;
		case SDLK_F2:
			new_game();
			break;
		case SDLK_F3:
			pause();
			break;
		case SDLK_F4:
			options::toggle(Menu1::Full_Screen);
			break;
		case SDLK_F5:
			options::toggle(Menu1::Sounds);
			break;
		case SDLK_F6:
			options::toggle(Menu1::Music);
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
		case SDLK_g:
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
		case SDL_WINDOWEVENT_SHOWN:
			activated = 1;
			Sound::Activate();
			if (options::Options.Music && !single_step)
				midi::play_pb_theme(0);
			no_time_loss = 1;
			has_focus = 1;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_HIDDEN:
			activated = 0;
			fullscrn::activate(0);
			options::Options.FullScreen = 0;
			Sound::Deactivate();
			midi::music_stop();
			has_focus = 0;
			pb::loose_focus();
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED:
			fullscrn::window_size_changed();
			break;
		default:;
		}
		break;
	default:;
	}

	return 1;
}

int winmain::ProcessWindowMessages()
{
	SDL_Event event;
#ifndef __EMSCRIPTEN__
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
#else
	while (SDL_PollEvent(&event))
	{
		if (!event_handler(&event))
			return 0;
	}

	return 1;
#endif
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
	char *caption = pinball::get_rc_string(170, 0);
	char *text = pinball::get_rc_string(179, 0);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, caption, text, MainWindow);
	std::exit(1);
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
		ImGui::TextUnformatted("Original game by Cinematronics, Microsoft");
		ImGui::Separator();

		ImGui::TextUnformatted("Decompiled -> Ported to SDL");
		if (ImGui::SmallButton("Project home: https://github.com/k4zmu2a/SpaceCadetPinball"))
		{
#if SDL_VERSION_ATLEAST(2, 0, 14)
			// Relatively new feature, skip with older SDL
			SDL_OpenURL("https://github.com/k4zmu2a/SpaceCadetPinball");
#endif
		}
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
	pb::replay_level(0);
}

void winmain::pause()
{
	pb::pause_continue();
	no_time_loss = 1;
}

void winmain::Restart()
{
	restart = true;
	SDL_Event event{SDL_QUIT};
	SDL_PushEvent(&event);
}

void winmain::UpdateFrameRate()
{
	// UPS >= FPS
	auto fps = options::Options.FramesPerSecond, ups = options::Options.UpdatesPerSecond;
	UpdateToFrameRatio = static_cast<double>(ups) / fps;
	TargetFrameTime = DurationMs(1000.0 / ups);
}
