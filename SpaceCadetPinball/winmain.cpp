#include "pch.h"
#include "winmain.h"

#include "control.h"
#include "fullscrn.h"
#include "midi.h"
#include "pinball.h"
#include "options.h"
#include "pb.h"
#include "render.h"
#include "Sound.h"

SDL_Window* winmain::MainWindow = nullptr;
SDL_Renderer* winmain::Renderer = nullptr;
ImGuiIO* winmain::ImIO = nullptr;

int winmain::return_value = 0;
bool winmain::bQuit = false;
bool winmain::activated = false;
int winmain::DispFrameRate = 0;
int winmain::DispGRhistory = 0;
bool winmain::single_step = false;
bool winmain::has_focus = true;
int winmain::last_mouse_x;
int winmain::last_mouse_y;
int winmain::mouse_down;
bool winmain::no_time_loss = false;

bool winmain::restart = false;

gdrv_bitmap8* winmain::gfr_display = nullptr;
std::string winmain::DatFileName;
bool winmain::ShowAboutDialog = false;
bool winmain::ShowImGuiDemo = false;
bool winmain::ShowSpriteViewer = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
std::string winmain::BasePath;
int winmain::MainMenuHeight = 0;
std::string winmain::FpsDetails;
double winmain::UpdateToFrameRatio;
winmain::DurationMs winmain::TargetFrameTime;
optionsStruct& winmain::Options = options::Options;

int winmain::WinMain(LPCSTR lpCmdLine)
{
	restart = false;
	bQuit = false;

	std::set_new_handler(memalloc_failure);

	// SDL init
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
		SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not initialize SDL2", SDL_GetError(), nullptr);
		return 1;
	}

	pinball::quickFlag = strstr(lpCmdLine, "-quick") != nullptr;

	// Search for game data in: game folder, user folder
	// Game data test order: CADET.DAT, PINBALL.DAT
	char* dataSearchPaths[2]
	{
		 SDL_GetBasePath(),
		 SDL_GetPrefPath(nullptr, "SpaceCadetPinball")
	};
	std::string datFileNames[2]
	{
		"CADET.DAT",
		options::get_string("Pinball Data", pinball::get_rc_string(168, 0))
	};
	for (auto path : dataSearchPaths)
	{
		if (DatFileName.empty() && path)
		{
			BasePath = path;
			for (int i = 0; i < 2; i++)
			{
				auto datFileName = datFileNames[i];
				auto datFilePath = pinball::make_path_name(datFileName);
				auto datFile = fopen(datFilePath.c_str(), "r");
				if (datFile)
				{
					fclose(datFile);
					DatFileName = datFileName;
					if (i == 0)
						pb::FullTiltMode = true;
					printf("Loading game from: %s\n", datFilePath.c_str());
					break;
				}
			}
		}

		SDL_free(path);
	}

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

	// If HW fails, fallback to SW SDL renderer.
	SDL_Renderer* renderer = nullptr;
	for (int i = 0; i < 2 && !renderer; i++)
	{
		Renderer = renderer = SDL_CreateRenderer
		(
			window,
			-1,
			i == 0 ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE
		);
	}
	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create renderer", SDL_GetError(), window);
		return 1;
	}
	SDL_RendererInfo rendererInfo{};
	if (!SDL_GetRendererInfo(renderer, &rendererInfo))
		printf("Using SDL renderer: %s\n", rendererInfo.name);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	// ImGui init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, 0, 0);
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	ImIO = &io;
	// ImGui_ImplSDL2_Init is private, we are not actually using ImGui OpenGl backend
	ImGui_ImplSDL2_InitForOpenGL(window, nullptr);

	auto prefPath = SDL_GetPrefPath(nullptr, "SpaceCadetPinball");
	auto iniPath = std::string(prefPath) + "imgui_pb.ini";
	io.IniFilename = iniPath.c_str();
	SDL_free(prefPath);

	// PB init from message handler
	{
		options::init();
		if (!Sound::Init(Options.SoundChannels, Options.Sounds))
			Options.Sounds = false;

		if (!pinball::quickFlag && !midi::music_init())
			Options.Music = false;

		if (pb::init())
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not load game data",
			                         "The .dat file is missing", window);
			return 1;
		}

		fullscrn::init();
	}

	pb::reset_table();
	pb::firsttime_setup();

	if (strstr(lpCmdLine, "-fullscreen"))
	{
		Options.FullScreen = true;
	}

	SDL_ShowWindow(window);
	fullscrn::set_screen_mode(Options.FullScreen);

	if (strstr(lpCmdLine, "-demo"))
		pb::toggle_demo();
	else
		pb::replay_level(0);

	unsigned dtHistoryCounter = 300u, updateCounter = 0, frameCounter = 0;

	auto frameStart = Clock::now();
	double UpdateToFrameCounter = 0;
	DurationMs sleepRemainder(0), frameDuration(TargetFrameTime);
	auto prevTime = frameStart;
	while (true)
	{
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
			if (!gfr_display)
			{
				auto plt = static_cast<ColorRgba*>(malloc(1024u));
				auto pltPtr = &plt[10]; // first 10 entries are system colors hardcoded in display_palette()
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
				gfr_display = new gdrv_bitmap8(400, 15, false);
			}

			if (!dtHistoryCounter)
			{
				dtHistoryCounter = 300;
				gdrv::copy_bitmap(render::vscreen, 300, 10, 0, 30, gfr_display, 0, 0);
				gdrv::fill_bitmap(gfr_display, 300, 10, 0, 0, 0);
			}
		}

		if (!ProcessWindowMessages() || bQuit)
			break;

		if (has_focus)
		{
			if (mouse_down)
			{
				int x, y, w, h;
				SDL_GetMouseState(&x, &y);
				SDL_GetWindowSize(window, &w, &h);
				float dx = static_cast<float>(last_mouse_x - x) / static_cast<float>(w);
				float dy = static_cast<float>(y - last_mouse_y) / static_cast<float>(h);
				pb::ballset(dx, dy);

				SDL_WarpMouseInWindow(window, last_mouse_x, last_mouse_y);

				// Mouse warp does not work over remote desktop or in some VMs
				//last_mouse_x = x;
				//last_mouse_y = y;
			}
			if (!single_step && !no_time_loss)
			{
				auto dt = static_cast<float>(frameDuration.count());
				auto dtWhole = static_cast<int>(std::round(dt));
				pb::frame(dt);
				if (gfr_display)
				{
					auto deltaTPal = dtWhole + 10;
					auto fillChar = static_cast<uint8_t>(deltaTPal);
					if (deltaTPal > 236)
					{
						fillChar = 1;
					}
					gdrv::fill_bitmap(gfr_display, 1, 10, 300 - dtHistoryCounter, 0, fillChar);
					--dtHistoryCounter;
				}
				updateCounter++;
			}
			no_time_loss = false;

			if (UpdateToFrameCounter >= UpdateToFrameRatio)
			{
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();
				RenderUi();

				SDL_RenderClear(renderer);
				render::PresentVScreen();

				ImGui::Render();
				ImGuiSDL::Render(ImGui::GetDrawData());

				SDL_RenderPresent(renderer);
				frameCounter++;
				UpdateToFrameCounter -= UpdateToFrameRatio;
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
			if (targetTimeDelta > DurationMs::zero() && !Options.UncappedUpdatesPerSecond)
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
			frameDuration = std::min<DurationMs>(DurationMs(frameEnd - frameStart), 2 * TargetFrameTime);
			frameStart = frameEnd;
			UpdateToFrameCounter++;
		}
	}

	delete gfr_display;
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
	// A minimal window with a button to prevent menu lockout.
	if (!Options.ShowMenu)
	{
		ImGui::SetNextWindowPos(ImVec2{});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10, 0});
		if (ImGui::Begin("main", nullptr,
		                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
		                 ImGuiWindowFlags_AlwaysAutoResize |
		                 ImGuiWindowFlags_NoMove))
		{
			ImGui::PushID(1);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
			if (ImGui::Button("Menu"))
			{
				options::toggle(Menu1::Show_Menu);
			}
			ImGui::PopStyleColor(1);
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	// No demo window in release to save space
#ifndef NDEBUG
	if (ShowImGuiDemo)
		ImGui::ShowDemoWindow(&ShowImGuiDemo);
#endif

	if (Options.ShowMenu && ImGui::BeginMainMenuBar())
	{
		int currentMenuHeight = static_cast<int>(ImGui::GetWindowSize().y);
		if (MainMenuHeight != currentMenuHeight)
		{
			// Get the height of the main menu bar and update screen coordinates
			MainMenuHeight = currentMenuHeight;
			fullscrn::window_size_changed();
		}

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
			if (ImGui::MenuItem("Exit"))
			{
				SDL_Event event{SDL_QUIT};
				SDL_PushEvent(&event);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Show Menu", "F9", Options.ShowMenu))
			{
				options::toggle(Menu1::Show_Menu);
			}
			if (ImGui::MenuItem("Full Screen", "F4", Options.FullScreen))
			{
				options::toggle(Menu1::Full_Screen);
			}
			if (ImGui::BeginMenu("Select Players"))
			{
				if (ImGui::MenuItem("1 Player", nullptr, Options.Players == 1))
				{
					options::toggle(Menu1::OnePlayer);
					new_game();
				}
				if (ImGui::MenuItem("2 Players", nullptr, Options.Players == 2))
				{
					options::toggle(Menu1::TwoPlayers);
					new_game();
				}
				if (ImGui::MenuItem("3 Players", nullptr, Options.Players == 3))
				{
					options::toggle(Menu1::ThreePlayers);
					new_game();
				}
				if (ImGui::MenuItem("4 Players", nullptr, Options.Players == 4))
				{
					options::toggle(Menu1::FourPlayers);
					new_game();
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Sound", "F5", Options.Sounds))
			{
				options::toggle(Menu1::Sounds);
			}
			if (ImGui::MenuItem("Music", "F6", Options.Music))
			{
				options::toggle(Menu1::Music);
			}
			ImGui::TextUnformatted("Sound Channels");
			if (ImGui::SliderInt("##Sound Channels", &Options.SoundChannels, options::MinSoundChannels,
			                     options::MaxSoundChannels, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				Options.SoundChannels = std::min(options::MaxSoundChannels,
				                                 std::max(options::MinSoundChannels, Options.SoundChannels));
				Sound::SetChannels(Options.SoundChannels);
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Player Controls...", "F8"))
			{
				if (!single_step)
					pause();
				options::ShowControlDialog();
			}
			if (ImGui::BeginMenu("Table Resolution"))
			{
				char buffer[20]{};
				auto maxResText = pinball::get_rc_string(fullscrn::GetMaxResolution() + 2030, 0);
				if (ImGui::MenuItem(maxResText, nullptr, Options.Resolution == -1))
				{
					options::toggle(Menu1::MaximumResolution);
				}
				for (auto i = 0; i <= fullscrn::GetMaxResolution(); i++)
				{
					auto& res = fullscrn::resolution_array[i];
					snprintf(buffer, sizeof buffer - 1, "%d x %d", res.ScreenWidth, res.ScreenHeight);
					if (ImGui::MenuItem(buffer, nullptr, Options.Resolution == i))
					{
						options::toggle(static_cast<Menu1>(static_cast<int>(Menu1::R640x480) + i));
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Graphics"))
			{
				if (ImGui::MenuItem("Uniform Scaling", nullptr, Options.UniformScaling))
				{
					options::toggle(Menu1::WindowUniformScale);
				}
				if (ImGui::MenuItem("Linear Filtering", nullptr, Options.LinearFiltering))
				{
					options::toggle(Menu1::WindowLinearFilter);
				}
				ImGui::DragFloat("UI Scale", &ImIO->FontGlobalScale, 0.005f, 0.8f, 5,
				                 "%.2f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Separator();

				char buffer[80]{};
				auto changed = false;
				if (ImGui::MenuItem("Set Default UPS/FPS"))
				{
					changed = true;
					Options.UpdatesPerSecond = options::DefUps;
					Options.FramesPerSecond = options::DefFps;
				}
				if (ImGui::SliderInt("UPS", &Options.UpdatesPerSecond, options::MinUps, options::MaxUps, "%d",
				                     ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					Options.FramesPerSecond = std::min(Options.UpdatesPerSecond, Options.FramesPerSecond);
				}
				if (ImGui::SliderInt("FPS", &Options.FramesPerSecond, options::MinFps, options::MaxFps, "%d",
				                     ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					Options.UpdatesPerSecond = std::max(Options.UpdatesPerSecond, Options.FramesPerSecond);
				}
				snprintf(buffer, sizeof buffer - 1, "Uncapped UPS (FPS ratio %02.02f)", UpdateToFrameRatio);
				if (ImGui::MenuItem(buffer, nullptr, Options.UncappedUpdatesPerSecond))
				{
					Options.UncappedUpdatesPerSecond ^= true;
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
#endif
			if (ImGui::MenuItem("Sprite Viewer", nullptr, ShowSpriteViewer))
			{
				if (!ShowSpriteViewer && !single_step)
					pause();
				ShowSpriteViewer ^= true;
			}
			if (ImGui::BeginMenu("Cheats"))
			{
				if (ImGui::MenuItem("hidden test", nullptr, pb::cheat_mode))
					pb::PushCheat("hidden test");
				if (ImGui::MenuItem("1max"))
					pb::PushCheat("1max");
				if (ImGui::MenuItem("bmax", nullptr, control::table_unlimited_balls))
					pb::PushCheat("bmax");
				if (ImGui::MenuItem("gmax"))
					pb::PushCheat("gmax");
				if (ImGui::MenuItem("rmax"))
					pb::PushCheat("rmax");
				if (pb::FullTiltMode && ImGui::MenuItem("quote"))
					pb::PushCheat("quote");

				ImGui::EndMenu();
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
		if (DispFrameRate && !FpsDetails.empty())
			if (ImGui::BeginMenu(FpsDetails.c_str()))
				ImGui::EndMenu();
		ImGui::EndMainMenuBar();
	}

	a_dialog();
	high_score::RenderHighScoreDialog();
	if (ShowSpriteViewer)
		render::SpriteViewer(&ShowSpriteViewer);
	options::RenderControlDialog();
}

int winmain::event_handler(const SDL_Event* event)
{
	ImGui_ImplSDL2_ProcessEvent(event);

	if (ImIO->WantCaptureMouse && !options::WaitingForInput())
	{
		if (mouse_down)
		{
			mouse_down = 0;
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
		bQuit = true;
		fullscrn::shutdown();
		return_value = 0;
		return 0;
	case SDL_KEYUP:
		pb::InputUp({InputTypes::Keyboard, event->key.keysym.sym});
		break;
	case SDL_KEYDOWN:
		if (!event->key.repeat)
			pb::InputDown({InputTypes::Keyboard, event->key.keysym.sym});
		switch (event->key.keysym.sym)
		{
		case SDLK_ESCAPE:
			if (Options.FullScreen)
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
			options::ShowControlDialog();
			break;
		case SDLK_F9:
			options::toggle(Menu1::Show_Menu);
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
			single_step ^= true;
			if (!single_step)
				no_time_loss = true;
			break;
		default:
			break;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		{
			bool noInput = false;
			switch (event->button.button)
			{
			case SDL_BUTTON_LEFT:
				if (pb::cheat_mode)
				{
					mouse_down = 1;
					last_mouse_x = event->button.x;
					last_mouse_y = event->button.y;
					SDL_SetWindowGrab(MainWindow, SDL_TRUE);
					noInput = true;
				}
				break;
			default:
				break;
			}

			if (!noInput)
				pb::InputDown({InputTypes::Mouse, event->button.button});
		}
		break;
	case SDL_MOUSEBUTTONUP:
		{
			bool noInput = false;
			switch (event->button.button)
			{
			case SDL_BUTTON_LEFT:
				if (mouse_down)
				{
					mouse_down = 0;
					SDL_SetWindowGrab(MainWindow, SDL_FALSE);
					noInput = true;
				}
				break;
			default:
				break;
			}

			if (!noInput)
				pb::InputUp({InputTypes::Mouse, event->button.button});
		}
		break;
	case SDL_WINDOWEVENT:
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_TAKE_FOCUS:
		case SDL_WINDOWEVENT_SHOWN:
			activated = true;
			Sound::Activate();
			if (Options.Music && !single_step)
				midi::play_pb_theme();
			no_time_loss = true;
			has_focus = true;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_HIDDEN:
			activated = false;
			fullscrn::activate(0);
			Options.FullScreen = false;
			Sound::Deactivate();
			midi::music_stop();
			has_focus = false;
			pb::loose_focus();
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED:
			fullscrn::window_size_changed();
			break;
		default: ;
		}
		break;
	case SDL_JOYDEVICEADDED:
		if (SDL_IsGameController(event->jdevice.which))
		{
			SDL_GameControllerOpen(event->jdevice.which);
		}
		break;
	case SDL_JOYDEVICEREMOVED:
		{
			SDL_GameController* controller = SDL_GameControllerFromInstanceID(event->jdevice.which);
			if (controller)
			{
				SDL_GameControllerClose(controller);
			}
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		pb::InputDown({InputTypes::GameController, event->cbutton.button});
		switch (event->cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_START:
			pause();
			break;
		case SDL_CONTROLLER_BUTTON_BACK:
			if (single_step)
			{
				SDL_Event event{ SDL_QUIT };
				SDL_PushEvent(&event);
			}
			break;
		default:;
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		pb::InputUp({InputTypes::GameController, event->cbutton.button});
		break;
	default: ;
	}

	return 1;
}

int winmain::ProcessWindowMessages()
{
	static auto idleWait = 0;
	SDL_Event event;
	if (has_focus && !single_step)
	{
		idleWait = static_cast<int>(TargetFrameTime.count());
		while (SDL_PollEvent(&event))
		{
			if (!event_handler(&event))
				return 0;
		}

		return 1;
	}

	// Progressively wait longer when transitioning to idle
	idleWait = std::min(idleWait + static_cast<int>(TargetFrameTime.count()), 500);
	if (SDL_WaitEventTimeout(&event, idleWait))
	{
		idleWait = static_cast<int>(TargetFrameTime.count());
		return event_handler(&event);
	}
	return 1;
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
	char* caption = pinball::get_rc_string(170, 0);
	char* text = pinball::get_rc_string(179, 0);
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
		ImGui::TextUnformatted("Version 2.0");
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
		no_time_loss = true;
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
	no_time_loss = true;
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
	auto fps = Options.FramesPerSecond, ups = Options.UpdatesPerSecond;
	UpdateToFrameRatio = static_cast<double>(ups) / fps;
	TargetFrameTime = DurationMs(1000.0 / ups);
}
