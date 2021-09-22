#pragma once

struct resolution_info
{
	int16_t ScreenWidth;
	int16_t ScreenHeight;
	int16_t TableWidth;
	int16_t TableHeight;
	int16_t ResolutionMenuId;
};

class fullscrn
{
public:
	static int screen_mode;
	static int display_changed;
	static const resolution_info resolution_array[3];
	static float ScaleX;
	static float ScaleY;
	static int OffsetX;
	static int OffsetY;

	static void init();
	static void shutdown();
	static int set_screen_mode(int isFullscreen);
	static void activate(int flag);
	static int GetResolution();
	static void SetResolution(int value);
	static int GetMaxResolution();
	static void window_size_changed();
private :
	static int resolution;

	static int enableFullscreen();
	static int disableFullscreen();
};
