#pragma once
#include "gdrv.h"

class winmain
{
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
	static void help_introduction();
	static void Restart();
	static bool RestartRequested() { return restart; }
private:
	static int return_value, bQuit, DispFrameRate, DispGRhistory, activated;
	static int has_focus, mouse_down, last_mouse_x, last_mouse_y, no_time_loss;
	static DWORD then, now;
	static gdrv_bitmap8 gfr_display;
	static bool restart;
	static bool ShowAboutDialog;
	static bool ShowImGuiDemo;
	static bool ShowSpriteViewer;

	static void RenderUi();
};
