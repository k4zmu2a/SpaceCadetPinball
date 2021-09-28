#pragma once
#include "gdrv.h"

struct SdlTickClock
{
	using duration = std::chrono::milliseconds;
	using rep = duration::rep;
	using period = duration::period;
	using time_point = std::chrono::time_point<SdlTickClock>;
	static constexpr bool is_steady = true;

	static time_point now() noexcept
	{
		return time_point{duration{SDL_GetTicks()}};
	}
};

struct SdlPerformanceClock
{
	using duration = std::chrono::duration<uint64_t, std::nano>;
	using rep = duration::rep;
	using period = duration::period;
	using time_point = std::chrono::time_point<SdlPerformanceClock>;
	static constexpr bool is_steady = true;

	static time_point now() noexcept
	{
		const auto freq = SDL_GetPerformanceFrequency();
		const auto ctr = SDL_GetPerformanceCounter();
		const auto whole = (ctr / freq) * period::den;
		const auto part = (ctr % freq) * period::den / freq;
		return time_point(duration(whole + part));
	}
};

class winmain
{
	using Clock = SdlPerformanceClock; // Or std::chrono::steady_clock.
	using DurationMs = std::chrono::duration<double, std::milli>;
	using TimePoint = std::chrono::time_point<Clock>;

public:
	static std::string DatFileName;
	static int single_step;
	static SDL_Window* MainWindow;
	static SDL_Renderer* Renderer;
	static ImGuiIO* ImIO;
	static bool LaunchBallEnabled;
	static bool HighScoresEnabled;
	static bool DemoActive;
	static char* BasePath;

	static int WinMain(LPCSTR lpCmdLine);
	static int event_handler(const SDL_Event* event);
	static void memalloc_failure();
	static int ProcessWindowMessages();
	static void a_dialog();
	static void end_pause();
	static void new_game();
	static void pause();
	static void Restart();
	static bool RestartRequested() { return restart; }
	static void UpdateFrameRate();
private:
	static int return_value, bQuit, DispFrameRate, DispGRhistory, activated;
	static int has_focus, mouse_down, last_mouse_x, last_mouse_y, no_time_loss;
	static gdrv_bitmap8 gfr_display;
	static std::string FpsDetails;
	static bool restart;
	static bool ShowAboutDialog;
	static bool ShowImGuiDemo;
	static bool ShowSpriteViewer;
	static double UpdateToFrameRatio;
	static DurationMs TargetFrameTime;

	static void RenderUi();
};
