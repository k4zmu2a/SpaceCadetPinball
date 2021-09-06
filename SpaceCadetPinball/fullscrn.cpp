#include "pch.h"
#include "fullscrn.h"


#include "options.h"
#include "pb.h"
#include "render.h"
#include "winmain.h"


int fullscrn::screen_mode;
int fullscrn::display_changed;

int fullscrn::resolution = 0;
int fullscrn::maxResolution = 0;
const resolution_info fullscrn::resolution_array[3] =
{
	{640, 480, 600, 416, 501},
	{800, 600, 752, 520, 502},
	{1024, 768, 960, 666, 503},
};
float fullscrn::ScaleX = 1;
float fullscrn::ScaleY = 1;
int fullscrn::OffsetX = 0;
int fullscrn::OffsetY = 0;

void fullscrn::init()
{
	window_size_changed();
}

void fullscrn::shutdown()
{
	if (display_changed)
		set_screen_mode(0);
}

int fullscrn::set_screen_mode(int isFullscreen)
{
	int result = isFullscreen;
	if (isFullscreen == screen_mode)
		return result;
	screen_mode = isFullscreen;
	if (isFullscreen)
	{
		enableFullscreen();
		result = 1;
	}
	else
	{
		disableFullscreen();
		result = 1;
	}
	return result;
}

int fullscrn::enableFullscreen()
{
	if (!display_changed)
	{
		SDL_SetWindowFullscreen(winmain::MainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
		display_changed = 1;
		if (display_changed)
			return 1;
	}
	return 0;
}

int fullscrn::disableFullscreen()
{
	if (display_changed)
	{
		SDL_SetWindowFullscreen(winmain::MainWindow, 0);
		display_changed = 0;
	}

	return 0;
}

void fullscrn::activate(int flag)
{
	if (screen_mode)
	{
		if (!flag)
		{
			set_screen_mode(0);
		}
	}
}

int fullscrn::GetResolution()
{
	return resolution;
}

void fullscrn::SetResolution(int resolution)
{
	if (!pb::FullTiltMode)
		resolution = 0;
	assertm(resolution >= 0 && resolution <= 2, "Resolution value out of bounds");
	fullscrn::resolution = resolution;
}

int fullscrn::GetMaxResolution()
{
	return maxResolution;
}

void fullscrn::SetMaxResolution(int resolution)
{
	assertm(resolution >= 0 && resolution <= 2, "Resolution value out of bounds");
	maxResolution = resolution;
}

int fullscrn::get_max_supported_resolution()
{
	if (!pb::FullTiltMode)
		return 0;

	int width = 0, height = 0;
	get_screen_resolution(&width, &height);
	auto result = 0;

	for (auto index = 1; index < 3; ++index)
	{
		auto resPtr = &resolution_array[index];
		if (resPtr->ScreenWidth <= width && resPtr->ScreenHeight <= height)
			result = index;
	}
	return result;
}

int fullscrn::get_screen_resolution(int* width, int* height)
{
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) == 0)
	{
		*width = dm.w;
		*height = dm.h;
		return 0;
	}
	return 1;
}

void fullscrn::window_size_changed()
{
	int width, height;
	SDL_GetRendererOutputSize(winmain::Renderer, &width, &height);
	auto res = &resolution_array[resolution];
	ScaleX = static_cast<float>(width) / res->TableWidth;
	ScaleY = static_cast<float>(height) / res->TableHeight;
	OffsetX = OffsetY = 0;

	if (options::Options.UniformScaling)
	{
		ScaleY = ScaleX = min(ScaleX, ScaleY);
		OffsetX = static_cast<int>(floor((width - res->TableWidth * ScaleX) / 2));
		OffsetY = static_cast<int>(floor((height - res->TableHeight * ScaleY) / 2));
	}

	gdrv::DestinationRect = SDL_Rect
	{
		OffsetX, OffsetY,
		width - OffsetX * 2, height - OffsetY * 2
	};
}
