#include "pch.h"
#include "winmain.h"

#include "control.h"
#include "fullscrn.h"
#include "midi.h"
#include "options.h"
#include "pb.h"
#include "render.h"
#include "Sound.h"
#include "translations.h"
#include "font_selection.h"

constexpr const char* winmain::Version;

SDL_Window* winmain::MainWindow = nullptr;
SDL_Renderer* winmain::Renderer = nullptr;
ImGuiIO* winmain::ImIO = nullptr;

int winmain::return_value = 0;
bool winmain::bQuit = false;
bool winmain::activated = false;
bool winmain::DispFrameRate = false;
bool winmain::DispGRhistory = false;
bool winmain::single_step = false;
bool winmain::has_focus = true;
int winmain::last_mouse_x;
int winmain::last_mouse_y;
int winmain::mouse_down;
bool winmain::no_time_loss = false;

bool winmain::restart = false;

std::vector<float> winmain::gfrDisplay{};
unsigned winmain::gfrOffset = 0;
float winmain::gfrWindow = 5.0f;
bool winmain::ShowAboutDialog = false;
bool winmain::ShowImGuiDemo = false;
bool winmain::ShowSpriteViewer = false;
bool winmain::ShowExitPopup = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
int winmain::MainMenuHeight = 0;
std::string winmain::FpsDetails, winmain::PrevSdlError;
unsigned winmain::PrevSdlErrorCount = 0;
double winmain::UpdateToFrameRatio;
winmain::DurationMs winmain::TargetFrameTime;
optionsStruct& winmain::Options = options::Options;
winmain::DurationMs winmain::SpinThreshold = DurationMs(0.005);
WelfordState winmain::SleepState{};

int winmain::WinMain(LPCSTR lpCmdLine)
{
	std::set_new_handler(memalloc_failure);

	printf("Game version: %s\n", Version);
	printf("Command line: %s\n", lpCmdLine);
	printf("Compiled with: SDL %d.%d.%d;", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
	printf(" SDL_mixer %d.%d.%d;", SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);
	printf(" ImGui %s %s\n", IMGUI_VERSION, ImGuiRender);

	// SDL init
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
		SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
	{
		pb::ShowMessageBox(SDL_MESSAGEBOX_ERROR, "Could not initialize SDL2", SDL_GetError());
		return 1;
	}

	pb::quickFlag = strstr(lpCmdLine, "-quick") != nullptr;

	// SDL window
	SDL_Window* window = SDL_CreateWindow
	(
		pb::get_rc_string(Msg::STRING139),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		800, 556,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE
	);
	MainWindow = window;
	if (!window)
	{
		pb::ShowMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", SDL_GetError());
		return 1;
	}

	// If HW fails, fallback to SW SDL renderer.
	SDL_Renderer* renderer = nullptr;
	auto swOffset = strstr(lpCmdLine, "-sw") != nullptr ? 1 : 0;
	for (int i = swOffset; i < 2 && !renderer; i++)
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
		pb::ShowMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create renderer", SDL_GetError());
		return 1;
	}
	SDL_RendererInfo rendererInfo{};
	if (!SDL_GetRendererInfo(renderer, &rendererInfo))
		printf("Using SDL renderer: %s\n", rendererInfo.name);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	auto prefPath = SDL_GetPrefPath("", "SpaceCadetPinball");
	auto basePath = SDL_GetBasePath();

	// SDL mixer init
	bool mixOpened = false, noAudio = strstr(lpCmdLine, "-noaudio") != nullptr;
	if (!noAudio)
	{
		if ((Mix_Init(MIX_INIT_MID_Proxy) & MIX_INIT_MID_Proxy) == 0)
		{
			printf("Could not initialize SDL MIDI, music might not work.\nSDL Error: %s\n", SDL_GetError());
			SDL_ClearError();
		}
		if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
		{
			printf("Could not open audio device, continuing without audio.\nSDL Error: %s\n", SDL_GetError());
			SDL_ClearError();
		}
		else
			mixOpened = true;
	}

	auto resetAllOptions = strstr(lpCmdLine, "-reset") != nullptr;
	do
	{
		restart = false;

		// ImGui init
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImIO = &io;
		auto iniPath = std::string(prefPath) + "imgui_pb.ini";
		io.IniFilename = iniPath.c_str();

		// First option initialization step: just load settings from .ini. Needs ImGui context.
		options::InitPrimary();
		if (resetAllOptions)
		{
			resetAllOptions = false;
			options::ResetAllOptions();
		}

		if (!Options.FontFileName.V.empty())
		{
			ImVector<ImWchar> ranges;
			translations::GetGlyphRange(&ranges);
			ImFontConfig fontConfig{};

			// ToDo: further tweak font options, maybe try imgui_freetype
			fontConfig.OversampleV = 2;
			fontConfig.OversampleH = 4;

			// ToDo: improve font file test, checking if file exists is not enough
			auto fontLoaded = false;
			auto fileName = Options.FontFileName.V.c_str();
			auto fileHandle = fopenu(fileName, "rb");
			if (fileHandle)
			{
				fclose(fileHandle);

				// ToDo: Bind font size to UI scale
				if (io.Fonts->AddFontFromFileTTF(fileName, 13.f, &fontConfig, ranges.Data))
					fontLoaded = true;
			}

			if (!fontLoaded)
				printf("Failed to load font: %s, using embedded font.\n", fileName);
			io.Fonts->Build();
		}
		ImGui_Render_Init(renderer);
		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForSDLRenderer(window, Renderer);
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

		// Data search order: WD, executable path, user pref path, platform specific paths.
		std::vector<const char*> searchPaths
		{
			{
				"",
				basePath,
				prefPath
			}
		};
		searchPaths.insert(searchPaths.end(), std::begin(PlatformDataPaths), std::end(PlatformDataPaths));
		pb::SelectDatFile(searchPaths);

		// Second step: run updates that depend on .DAT file selection
		options::InitSecondary();

		Sound::Init(mixOpened, Options.SoundChannels, Options.Sounds, Options.SoundVolume);
		if (!mixOpened)
			Options.Sounds = false;

		if (!midi::music_init(mixOpened, Options.MusicVolume))
			Options.Music = false;

		if (pb::init())
		{
			std::string message = "The .dat file is missing.\n"
				"Make sure that the game data is present in any of the following locations:\n";
			for (auto path : searchPaths)
			{
				if (path)
				{
					message = message + (path[0] ? path : "working directory") + "\n";
				}
			}
			pb::ShowMessageBox(SDL_MESSAGEBOX_ERROR, "Could not load game data", message.c_str());
			return 1;
		}

		fullscrn::init();

		pb::reset_table();
		pb::firsttime_setup();

		if (strstr(lpCmdLine, "-fullscreen"))
		{
			Options.FullScreen = true;
		}

		if (!Options.FullScreen)
		{
			auto resInfo = &fullscrn::resolution_array[fullscrn::GetResolution()];
			SDL_SetWindowSize(MainWindow, resInfo->TableWidth, resInfo->TableHeight);
		}
		SDL_ShowWindow(window);
		fullscrn::set_screen_mode(Options.FullScreen);

		if (strstr(lpCmdLine, "-demo"))
			pb::toggle_demo();
		else
			pb::replay_level(false);

		MainLoop();

		options::uninit();
		midi::music_shutdown();
		Sound::Close();
		pb::uninit();

		ImGui_Render_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}
	while (restart);

	if (!noAudio)
	{
		if (mixOpened)
			Mix_CloseAudio();
		Mix_Quit();
	}

	SDL_free(basePath);
	SDL_free(prefPath);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return return_value;
}

void winmain::MainLoop()
{
	bQuit = false;
	unsigned updateCounter = 0, frameCounter = 0;
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
				SDL_SetWindowTitle(MainWindow, buf);
				FpsDetails = buf;
				frameCounter = updateCounter = 0;
				prevTime = curTime;
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
				SDL_GetWindowSize(MainWindow, &w, &h);
				float dx = static_cast<float>(last_mouse_x - x) / static_cast<float>(w);
				float dy = static_cast<float>(y - last_mouse_y) / static_cast<float>(h);
				pb::ballset(dx, dy);

				// Original creates continuous mouse movement with mouse capture.
				// Alternative solution: mouse warp at window edges.
				int xMod = 0, yMod = 0;
				if (x == 0 || x >= w - 1)
					xMod = w - 2;
				if (y == 0 || y >= h - 1)
					yMod = h - 2;
				if (xMod != 0 || yMod != 0)
				{
					// Mouse warp does not work over remote desktop or in some VMs
					x = abs(x - xMod);
					y = abs(y - yMod);
					SDL_WarpMouseInWindow(MainWindow, x, y);
				}

				last_mouse_x = x;
				last_mouse_y = y;
			}
			if (!single_step && !no_time_loss)
			{
				auto dt = static_cast<float>(frameDuration.count());
				pb::frame(dt);
				if (DispGRhistory)
				{
					auto targetSize = static_cast<unsigned>(static_cast<float>(Options.UpdatesPerSecond) * gfrWindow);
					if (gfrDisplay.size() != targetSize)
					{
						gfrDisplay.resize(targetSize, static_cast<float>(TargetFrameTime.count()));
						gfrOffset = 0;
					}
					gfrDisplay[gfrOffset] = dt;
					gfrOffset = (gfrOffset + 1) % gfrDisplay.size();
				}
				updateCounter++;
			}
			no_time_loss = false;

			if (UpdateToFrameCounter >= UpdateToFrameRatio)
			{
				ImGui_ImplSDL2_NewFrame();
				ImGui_Render_NewFrame();
				ImGui::NewFrame();
				RenderUi();

				SDL_RenderClear(Renderer);
				// Alternative clear hack, clear might fail on some systems
				// Todo: remove original clear, if save for all platforms
				SDL_RenderFillRect(Renderer, nullptr);
				render::PresentVScreen();

				ImGui::Render();
				ImGui_Render_RenderDrawData(ImGui::GetDrawData());

				SDL_RenderPresent(Renderer);
				frameCounter++;
				UpdateToFrameCounter -= UpdateToFrameRatio;
			}

			auto sdlError = SDL_GetError();
			if (sdlError[0] || !PrevSdlError.empty())
			{
				if (sdlError[0])
					SDL_ClearError();

				// Rate limit duplicate SDL error messages.
				if (sdlError != PrevSdlError)
				{
					PrevSdlError = sdlError;
					if (PrevSdlErrorCount > 0)
					{
						printf("SDL Error: ^ Previous Error Repeated %u Times\n", PrevSdlErrorCount + 1);
						PrevSdlErrorCount = 0;
					}

					if (sdlError[0])
						printf("SDL Error: %s\n", sdlError);
				}
				else
				{
					PrevSdlErrorCount++;
				}
			}

			auto updateEnd = Clock::now();
			auto targetTimeDelta = TargetFrameTime - DurationMs(updateEnd - frameStart) - sleepRemainder;

			TimePoint frameEnd;
			if (targetTimeDelta > DurationMs::zero() && !Options.UncappedUpdatesPerSecond)
			{
				if (Options.HybridSleep)
					HybridSleep(targetTimeDelta);
				else
					std::this_thread::sleep_for(targetTimeDelta);
				frameEnd = Clock::now();
			}
			else
			{
				frameEnd = updateEnd;
			}

			// Limit duration to 2 * target time
			sleepRemainder = Clamp(DurationMs(frameEnd - updateEnd) - targetTimeDelta, -TargetFrameTime,
			                       TargetFrameTime);
			frameDuration = std::min<DurationMs>(DurationMs(frameEnd - frameStart), 2 * TargetFrameTime);
			frameStart = frameEnd;
			UpdateToFrameCounter++;
		}
	}

	if (PrevSdlErrorCount > 0)
	{
		printf("SDL Error: ^ Previous Error Repeated %u Times\n", PrevSdlErrorCount);
	}
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
		                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing))
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

		// This window can not loose nav focus for some reason, clear it manually.
		if (ImGui::IsKeyDown(ImGuiKey_Escape) || ImGui::IsKeyDown(ImGuiKey_GamepadFaceRight))
			ImGui::FocusWindow(nullptr);
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

		if (ImGui::BeginMenu(pb::get_rc_string(Msg::Menu1_Game)))
		{
			ImGuiMenuItemWShortcut(GameBindings::NewGame);
			if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_Launch_Ball), nullptr, false, LaunchBallEnabled))
			{
				end_pause();
				pb::launch_ball();
			}
			ImGuiMenuItemWShortcut(GameBindings::TogglePause);
			ImGui::Separator();

			if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_High_Scores), nullptr, false, HighScoresEnabled))
			{
				pause(false);
				pb::high_scores();
			}
			if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_Demo), nullptr, DemoActive))
			{
				end_pause();
				pb::toggle_demo();
			}
			ImGuiMenuItemWShortcut(GameBindings::Exit);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(pb::get_rc_string(Msg::Menu1_Options)))
		{
			ImGuiMenuItemWShortcut(GameBindings::ToggleMenuDisplay, Options.ShowMenu);
			ImGuiMenuItemWShortcut(GameBindings::ToggleFullScreen, Options.FullScreen);
			if (ImGui::BeginMenu(pb::get_rc_string(Msg::Menu1_Select_Players)))
			{
				if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_1Player), nullptr, Options.Players == 1))
				{
					options::toggle(Menu1::OnePlayer);
					new_game();
				}
				if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_2Players), nullptr, Options.Players == 2))
				{
					options::toggle(Menu1::TwoPlayers);
					new_game();
				}
				if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_3Players), nullptr, Options.Players == 3))
				{
					options::toggle(Menu1::ThreePlayers);
					new_game();
				}
				if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_4Players), nullptr, Options.Players == 4))
				{
					options::toggle(Menu1::FourPlayers);
					new_game();
				}
				ImGui::EndMenu();
			}
			ImGuiMenuItemWShortcut(GameBindings::ShowControlDialog);
			if (ImGui::BeginMenu("Language"))
			{
				auto currentLanguage = translations::GetCurrentLanguage();
				for (auto& item : translations::Languages)
				{
					if (ImGui::MenuItem(item.DisplayName, nullptr, currentLanguage->Language == item.Language))
					{
						if (currentLanguage->Language != item.Language)
						{
							translations::SetCurrentLanguage(item.ShortName);
							Restart();
						}
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();

			if (ImGui::BeginMenu("Audio"))
			{
				ImGuiMenuItemWShortcut(GameBindings::ToggleSounds, Options.Sounds);
				if (ImGui::MenuItem("Stereo Sound Effects", nullptr, Options.SoundStereo))
				{
					options::toggle(Menu1::SoundStereo);
				}
				ImGui::TextUnformatted("Sound Volume");
				if (ImGui::SliderInt("##Sound Volume", &Options.SoundVolume.V, options::MinVolume, options::MaxVolume,
				                     "%d",
				                     ImGuiSliderFlags_AlwaysClamp))
				{
					Sound::SetVolume(Options.SoundVolume);
				}
				ImGui::TextUnformatted("Sound Channels");
				if (ImGui::SliderInt("##Sound Channels", &Options.SoundChannels.V, options::MinSoundChannels,
				                     options::MaxSoundChannels, "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					Sound::SetChannels(Options.SoundChannels);
				}
				ImGui::Separator();

				ImGuiMenuItemWShortcut(GameBindings::ToggleMusic, Options.Music);
				ImGui::TextUnformatted("Music Volume");
				if (ImGui::SliderInt("##Music Volume", &Options.MusicVolume.V, options::MinVolume, options::MaxVolume,
				                     "%d",
				                     ImGuiSliderFlags_AlwaysClamp))
				{
					midi::SetVolume(Options.MusicVolume);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Graphics"))
			{
				if (ImGui::MenuItem("Change Font"))
				{
					font_selection::ShowDialog();
				}
				if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_WindowUniformScale), nullptr, Options.UniformScaling))
				{
					options::toggle(Menu1::WindowUniformScale);
				}
				if (ImGui::MenuItem("Linear Filtering", nullptr, Options.LinearFiltering))
				{
					options::toggle(Menu1::WindowLinearFilter);
				}
				if (ImGui::MenuItem("Integer Scaling", nullptr, Options.IntegerScaling))
				{
					options::toggle(Menu1::WindowIntegerScale);
				}
				if (ImGui::DragFloat("UI Scale", &Options.UIScale.V, 0.005f, 0.8f, 5,
				                     "%.2f", ImGuiSliderFlags_AlwaysClamp))
				{
					ImIO->FontGlobalScale = Options.UIScale;
				}
				ImGui::Separator();

				char buffer[80]{};
				auto changed = false;
				if (ImGui::MenuItem("Set Default UPS/FPS"))
				{
					changed = true;
					Options.UpdatesPerSecond = options::DefUps;
					Options.FramesPerSecond = options::DefFps;
				}
				if (ImGui::SliderInt("UPS", &Options.UpdatesPerSecond.V, options::MinUps, options::MaxUps, "%d",
				                     ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					Options.FramesPerSecond = std::min(Options.UpdatesPerSecond.V, Options.FramesPerSecond.V);
				}
				if (ImGui::SliderInt("FPS", &Options.FramesPerSecond.V, options::MinFps, options::MaxFps, "%d",
				                     ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					Options.UpdatesPerSecond = std::max(Options.UpdatesPerSecond.V, Options.FramesPerSecond.V);
				}
				snprintf(buffer, sizeof buffer - 1, "Uncapped UPS (FPS ratio %02.02f)", UpdateToFrameRatio);
				if (ImGui::MenuItem(buffer, nullptr, Options.UncappedUpdatesPerSecond))
				{
					Options.UncappedUpdatesPerSecond ^= true;
				}
				if (ImGui::MenuItem("Precise Sleep", nullptr, Options.HybridSleep))
				{
					Options.HybridSleep ^= true;
					SleepState = WelfordState{};
					SpinThreshold = DurationMs::zero();
				}

				if (changed)
				{
					UpdateFrameRate();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(pb::get_rc_string(Msg::Menu1_Table_Resolution)))
			{
				char buffer[20]{};
				auto resolutionStringId = Msg::Menu1_UseMaxResolution_640x480;

				switch (fullscrn::GetMaxResolution())
				{
				case 0: resolutionStringId = Msg::Menu1_UseMaxResolution_640x480;
					break;
				case 1: resolutionStringId = Msg::Menu1_UseMaxResolution_800x600;
					break;
				case 2: resolutionStringId = Msg::Menu1_UseMaxResolution_1024x768;
					break;
				}

				auto maxResText = pb::get_rc_string(resolutionStringId);
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

			if (ImGui::BeginMenu("Game Data"))
			{
				if (ImGui::MenuItem("Prefer 3DPB Data", nullptr, Options.Prefer3DPBGameData))
				{
					options::toggle(Menu1::Prefer3DPBGameData);
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Reset All Options"))
			{
				options::ResetAllOptions();
				Restart();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(pb::get_rc_string(Msg::Menu1_Help)))
		{
#ifndef NDEBUG
			if (ImGui::MenuItem("ImGui Demo", nullptr, ShowImGuiDemo))
			{
				ShowImGuiDemo ^= true;
			}
#endif
			if (ImGui::MenuItem("Sprite Viewer", nullptr, ShowSpriteViewer))
			{
				if (!ShowSpriteViewer)
					pause(false);
				ShowSpriteViewer ^= true;
			}
			if (pb::cheat_mode && ImGui::MenuItem("Frame Times", nullptr, DispGRhistory))
			{
				DispGRhistory ^= true;
			}
			if (ImGui::MenuItem("Debug Overlay", nullptr, Options.DebugOverlay))
			{
				Options.DebugOverlay ^= true;
			}
			if (Options.DebugOverlay && ImGui::BeginMenu("Overlay Options"))
			{
				if (ImGui::MenuItem("Box Grid", nullptr, Options.DebugOverlayGrid))
					Options.DebugOverlayGrid ^= true;
				if (ImGui::MenuItem("Ball Depth Grid", nullptr, Options.DebugOverlayBallDepthGrid))
					Options.DebugOverlayBallDepthGrid ^= true;
				if (ImGui::MenuItem("Sprite Positions", nullptr, Options.DebugOverlaySprites))
					Options.DebugOverlaySprites ^= true;
				if (ImGui::MenuItem("All Edges", nullptr, Options.DebugOverlayAllEdges))
					Options.DebugOverlayAllEdges ^= true;
				if (ImGui::MenuItem("Component AABB", nullptr, Options.DebugOverlayAabb))
					Options.DebugOverlayAabb ^= true;
				if (ImGui::MenuItem("Ball Position", nullptr, Options.DebugOverlayBallPosition))
					Options.DebugOverlayBallPosition ^= true;
				if (ImGui::MenuItem("Ball Box Edges", nullptr, Options.DebugOverlayBallEdges))
					Options.DebugOverlayBallEdges ^= true;
				if (ImGui::MenuItem("Sound Positions", nullptr, Options.DebugOverlaySounds))
					Options.DebugOverlaySounds ^= true;
				if (ImGui::MenuItem("Apply Collision Mask", nullptr, Options.DebugOverlayCollisionMask))
					Options.DebugOverlayCollisionMask ^= true;
				ImGui::EndMenu();
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
				if (ImGui::MenuItem("easy mode", nullptr, control::easyMode))
					pb::PushCheat("easy mode");

				ImGui::EndMenu();
			}
			ImGui::Separator();

			if (ImGui::MenuItem(pb::get_rc_string(Msg::Menu1_About_Pinball)))
			{
				pause(false);
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
	font_selection::RenderDialog();
	if (ShowSpriteViewer)
		render::SpriteViewer(&ShowSpriteViewer);
	options::RenderControlDialog();
	if (DispGRhistory)
		RenderFrameTimeDialog();

	const auto exitText = translations::GetTranslation(Msg::Menu1_Exit);
	if (ShowExitPopup)
	{
		ShowExitPopup = false;
		pause(false);
		ImGui::OpenPopup(exitText);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	}
	if (ImGui::BeginPopupModal(exitText, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Exit the game?");
		ImGui::Separator();

		if (ImGui::Button(pb::get_rc_string(Msg::GenericOk), ImVec2(120, 0)))
		{
			SDL_Event event{SDL_QUIT};
			SDL_PushEvent(&event);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::IsWindowAppearing())
		{
			ImGui::SetKeyboardFocusHere(0);
		}
		if (ImGui::Button(pb::get_rc_string(Msg::GenericCancel), ImVec2(120, 0)))
		{
			end_pause();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	// Print game texts on the sidebar
	gdrv::grtext_draw_ttext_in_box();
}

int winmain::event_handler(const SDL_Event* event)
{
	auto inputDown = false;
	switch (event->type)
	{
	case SDL_KEYDOWN:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_CONTROLLERBUTTONDOWN:
		inputDown = true;
		break;
	default: break;
	}
	if (!options::WaitingForInput() || !inputDown)
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
	if (ImIO->WantCaptureKeyboard && !options::WaitingForInput())
	{
		switch (event->type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
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
		if (event->key.repeat)
			break;

		pb::InputDown({InputTypes::Keyboard, event->key.keysym.sym});
		if (!pb::cheat_mode)
			break;

		switch (event->key.keysym.sym)
		{
		case SDLK_g:
			DispGRhistory ^= true;
			break;
		case SDLK_o:
			{
				auto plt = new ColorRgba[4 * 256];
				auto pltPtr = &plt[10]; // first 10 entries are system colors hardcoded in display_palette()
				for (int i1 = 0, i2 = 0; i1 < 256 - 10; ++i1, i2 += 8)
				{
					unsigned char blue = i2, redGreen = i2;
					if (i2 > 255)
					{
						blue = 255;
						redGreen = i1;
					}

					*pltPtr++ = ColorRgba{blue, redGreen, redGreen, 0};
				}
				gdrv::display_palette(plt);
				delete[] plt;
			}
			break;
		case SDLK_y:
			SDL_SetWindowTitle(MainWindow, "Pinball");
			DispFrameRate ^= true;
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
				midi::music_play();
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
	if (has_focus)
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
	const char* caption = pb::get_rc_string(Msg::STRING270);
	const char* text = pb::get_rc_string(Msg::STRING279);
	pb::ShowMessageBox(SDL_MESSAGEBOX_ERROR, caption, text);
	std::exit(1);
}

void winmain::a_dialog()
{
	if (ShowAboutDialog == true)
	{
		ShowAboutDialog = false;
		ImGui::OpenPopup(pb::get_rc_string(Msg::STRING204));
	}

	bool unused_open = true;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2{ 600, 300 });
	if (ImGui::BeginPopupModal(pb::get_rc_string(Msg::STRING204), &unused_open, ImGuiWindowFlags_None))
	{
		if (ImGui::BeginTabBar("AboutTabBar", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("3DPB"))
			{
				ImGui::TextUnformatted(pb::get_rc_string(Msg::STRING139));
				ImGui::TextUnformatted("Original game by Cinematronics, Microsoft");
				ImGui::Separator();

				ImGui::TextUnformatted("Decompiled -> Ported to SDL");
				ImGui::Text("Version %s", Version);
				if (ImGui::SmallButton("Project home: https://github.com/k4zmu2a/SpaceCadetPinball"))
				{
#if SDL_VERSION_ATLEAST(2, 0, 14)
					// Relatively new feature, skip with older SDL
					SDL_OpenURL("https://github.com/k4zmu2a/SpaceCadetPinball");
#endif
				}
				ImGui::EndTabItem();
			}
			ImGui::PushStyleColor(ImGuiCol_Button, 0);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0);
			if (ImGui::BeginTabItem("Full Tilt!"))
			{
				const ImVec2 buttonCenter = { -1, 0 };
				ImGui::Button("Full Tilt! was created by Cinematronics for Maxis.", buttonCenter);
				ImGui::Button("Version 1.1", buttonCenter);

				auto tableRow = [](LPCSTR textA, LPCSTR textB)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(textA);
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(textB);
				};
				if (ImGui::BeginTable("Full Tilt!", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Button("Cinematronics", buttonCenter);
					ImGui::Separator();
					if (ImGui::BeginTable("Cinematronics", 2))
					{
						tableRow("PROGRAMMING", "ART");
						tableRow("Michael Sandige", "John Frantz");
						tableRow("John Taylor", "Ryan Medeiros");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Cinematronics", 2))
					{
						tableRow("DESIGN", "SOUND EFFECTS");
						tableRow("Kevin Gliner", "Matt Ridgeway");
						tableRow(nullptr, "Donald S. Griffin");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Cinematronics", 2))
					{
						tableRow("DESIGN CONSULTANT", "MUSIC");
						tableRow("Mark Sprenger", "Matt Ridgeway");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Cinematronics", 2))
					{
						tableRow("PRODUCER", "VOICES");
						tableRow("Kevin Gliner", "Mike McGeary");
						tableRow(nullptr, "William Rice");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Cinematronics", 2))
					{
						tableRow("GRAND POOBAH", nullptr);
						tableRow("David Stafford", nullptr);
						ImGui::EndTable();
					}
					ImGui::Separator();
					ImGui::Button("SPECIAL THANKS", buttonCenter);
					if (ImGui::BeginTable("Cinematronics", 2))
					{
						tableRow("Paula Sandige", "Alex St. John");
						tableRow("Brad Silverberg", "Jeff Camp");
						tableRow("Danny Thorpe", "Greg Hospelhorn");
						tableRow("Keith Johnson", "Sean Grant");
						tableRow("Bob McAnn", "Michael Kelley");
						tableRow("Rob Rosenhouse", "Lisa Acton");
						ImGui::EndTable();
					}
					ImGui::TextUnformatted("Dan and Mitchell Roth");

					ImGui::TableNextColumn();
					ImGui::Button("Maxis", buttonCenter);
					ImGui::Separator();
					if (ImGui::BeginTable("Maxis", 2))
					{
						tableRow("PRODUCER", "PRODUCT MANAGER");
						tableRow("John Csicsery", "Larry Lee");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Maxis", 2))
					{
						tableRow("LEAD TESTER", "QA MANAGER");
						tableRow("Scott Shicoff", "Scott Shicoff");
						ImGui::EndTable();
					}
					ImGui::Separator();
					ImGui::Button("ADDITIONAL TESTING", buttonCenter);
					if (ImGui::BeginTable("Maxis", 2))
					{
						tableRow("Cathy Castro", "Robin Hines");
						tableRow("John \"Jussi\" Ylinen", "Keith Meyer");
						tableRow("Marc Meyer", "Owen Nelson");
						tableRow("Joe Longworth", "Peter Saylor");
						tableRow("Michael Gilmartin", "Robin Hines");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Maxis", 2))
					{
						tableRow("ADDITIONAL ART", "ART DIRECTOR");
						tableRow("Ocean Quigley", "Sharon Barr");
						tableRow("Rick Macaraeg", "INSTALL PROGRAM");
						tableRow("Charlie Aquilina", "Kevin O'Hare");
						ImGui::EndTable();
					}
					ImGui::Separator();
					if (ImGui::BeginTable("Maxis", 2))
					{
						tableRow("INTRO MUSIC", "DOCUMENTATION");
						tableRow("Brian Conrad", "David Caggiano");
						tableRow("John Csicsery", "Michael Bremer");
						tableRow(nullptr, "Bob Sombrio");
						ImGui::EndTable();
					}
					ImGui::Separator();
					ImGui::Button("SPECIAL THANKS", buttonCenter);
					if (ImGui::BeginTable("Maxis", 2))
					{
						tableRow("Sam Poole", "Joe Scirica");
						tableRow("Jeff Braun", "Bob Derber");
						tableRow("Ashley Csicsery", "Tom Forge");
						ImGui::EndTable();
					}
					ImGui::Button("Will \"Burr\" Wright", buttonCenter);
					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
			ImGui::PopStyleColor(3);
		}

		ImGui::Separator();
		if (ImGui::Button("Ok"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
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
	pb::replay_level(false);
}

void winmain::pause(bool toggle)
{
	if (toggle || !single_step)
	{
		pb::pause_continue();
		no_time_loss = true;
	}
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
	auto fps = Options.FramesPerSecond.V, ups = Options.UpdatesPerSecond.V;
	UpdateToFrameRatio = static_cast<double>(ups) / fps;
	TargetFrameTime = DurationMs(1000.0 / ups);
}

void winmain::HandleGameBinding(GameBindings binding, bool shortcut)
{
	switch (binding)
	{
	case GameBindings::TogglePause:
		pause();
		break;
	case GameBindings::NewGame:
		new_game();
		break;
	case GameBindings::ToggleFullScreen:
		options::toggle(Menu1::Full_Screen);
		break;
	case GameBindings::ToggleSounds:
		options::toggle(Menu1::Sounds);
		break;
	case GameBindings::ToggleMusic:
		options::toggle(Menu1::Music);
		break;
	case GameBindings::ShowControlDialog:
		pause(false);
		options::ShowControlDialog();
		break;
	case GameBindings::ToggleMenuDisplay:
		options::toggle(Menu1::Show_Menu);
		break;
	case GameBindings::Exit:
		if (!shortcut)
		{
			SDL_Event event{SDL_QUIT};
			SDL_PushEvent(&event);
		}
		else
			ShowExitPopup = true;
		break;
	default:
		break;
	}
}

void winmain::RenderFrameTimeDialog()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2{300, 70});
	if (ImGui::Begin("Frame Times", &DispGRhistory, ImGuiWindowFlags_NoScrollbar))
	{
		auto target = static_cast<float>(TargetFrameTime.count());
		auto yMax = target * 2;

		auto spin = Options.HybridSleep ? static_cast<float>(SpinThreshold.count()) : 0;
		ImGui::Text("YMin:0ms, Target frame time:%03.04fms, YMax:%03.04fms, SpinThreshold:%03.04fms",
		            target, yMax, spin);

		static bool scrollPlot = true;
		ImGui::Checkbox("Scroll Plot", &scrollPlot);

		ImGui::SameLine();
		ImGui::SliderFloat("Window Size", &gfrWindow, 0.1f, 15, "%.3fsec", ImGuiSliderFlags_AlwaysClamp);

		{
			float average = 0.0f, dev = 0.0f;
			for (auto n : gfrDisplay)
			{
				average += n;
				dev += std::abs(target - n);
			}
			average /= static_cast<float>(gfrDisplay.size());
			dev /= static_cast<float>(gfrDisplay.size());
			char overlay[64];
			sprintf(overlay, "avg %.3fms, dev %.3fms", average, dev);

			auto region = ImGui::GetContentRegionAvail();
			ImGui::PlotLines("Lines", gfrDisplay.data(), gfrDisplay.size(),
			                 scrollPlot ? gfrOffset : 0, overlay, 0, yMax, region);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void winmain::HybridSleep(DurationMs sleepTarget)
{
	static constexpr double StdDevFactor = 0.5;

	// This nice concept is from https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
	// Sacrifices some CPU time for smaller frame time jitter
	while (sleepTarget > SpinThreshold)
	{
		auto start = Clock::now();
		std::this_thread::sleep_for(DurationMs(1));
		auto end = Clock::now();

		auto actualDuration = DurationMs(end - start);
		sleepTarget -= actualDuration;

		// Update expected sleep duration using Welford's online algorithm
		// With bad timer, this will run away to 100% spin
		SleepState.Advance(actualDuration.count());
		SpinThreshold = DurationMs(SleepState.mean + SleepState.GetStdDev() * StdDevFactor);
	}

	// spin lock
	for (auto start = Clock::now(); DurationMs(Clock::now() - start) < sleepTarget;);
}

void winmain::ImGuiMenuItemWShortcut(GameBindings binding, bool selected)
{
	const auto& keyDef = Options.Key[~binding];
	if (ImGui::MenuItem(pb::get_rc_string(keyDef.Description), keyDef.GetShortcutDescription().c_str(), selected))
	{
		HandleGameBinding(binding, false);
	}
}
