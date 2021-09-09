#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pb.h"
#include "resource.h"
#include "Sound.h"
#include "winmain.h"

LPCSTR options::OptionsRegPath;
LPSTR options::OptionsRegPathCur;
optionsStruct options::Options{};

winhelp_entry options::keymap_help[18]
{
	winhelp_entry{0x1F5, 0x3EA},
	winhelp_entry{0x191, 0x3EC},
	winhelp_entry{0x192, 0x3ED},
	winhelp_entry{0x193, 0x3F1},
	winhelp_entry{0x194, 0x3EE},
	winhelp_entry{0x195, 0x3EF},
	winhelp_entry{0x196, 0x3F0},
	winhelp_entry{0x385, 0x3EC},
	winhelp_entry{0x386, 0x3ED},
	winhelp_entry{0x387, 0x3F1},
	winhelp_entry{0x388, 0x3EE},
	winhelp_entry{0x389, 0x3EF},
	winhelp_entry{0x38A, 0x3F0},
	winhelp_entry{0x38C, -1},
	winhelp_entry{0x38D, -1},
	winhelp_entry{0x321, -1},
	winhelp_entry{0x322, -1},
	winhelp_entry{0, 0},
};

short options::vk_list[28]
{
	-32703,
	0x5A,
	-32720,
	0x39,
	0x402E,
	0x402F,
	0x403B,
	0x4027,
	0x405B,
	0x405D,
	0x20,
	0x0D,
	0x9,
	0x14,
	0x25,
	0x27,
	0x26,
	0x28,
	0x2D,
	0x2E,
	0x24,
	0x23,
	0x21,
	0x22,
	0x90,
	0x91,
	0x13,
	-1
};

void options::init()
{
	Options.Sounds = 1;
	Options.Music = 0;
	Options.FullScreen = 0;
	Options.Average = 5;
	Options.LeftFlipperKeyDft = SDLK_z;
	Options.RightFlipperKeyDft = SDLK_SLASH;
	Options.PlungerKeyDft = SDLK_SPACE;
	Options.LeftTableBumpKeyDft = SDLK_x;
	Options.RightTableBumpKeyDft = SDLK_PERIOD;
	Options.BottomTableBumpKeyDft = SDLK_UP;
	/*pinball::get_rc_int(159, &Options.LeftFlipperKeyDft);
	pinball::get_rc_int(160, &Options.RightFlipperKeyDft);
	pinball::get_rc_int(161, &Options.PlungerKeyDft);
	pinball::get_rc_int(162, &Options.LeftTableBumpKeyDft);
	pinball::get_rc_int(163, &Options.RightTableBumpKeyDft);
	pinball::get_rc_int(164, &Options.BottomTableBumpKeyDft);*/
	Options.LeftFlipperKey = Options.LeftFlipperKeyDft;
	Options.RightFlipperKey = Options.RightFlipperKeyDft;
	Options.PlungerKey = Options.PlungerKeyDft;
	Options.LeftTableBumpKey = Options.LeftTableBumpKeyDft;
	Options.RightTableBumpKey = Options.RightTableBumpKeyDft;
	Options.Players = 1;
	Options.BottomTableBumpKey = Options.BottomTableBumpKeyDft;
	Options.UniformScaling = true;
	/*Options.Sounds = get_int(nullptr, "Sounds", Options.Sounds);
	Options.Music = get_int(nullptr, "Music", Options.Music);
	Options.Average = get_int(nullptr, "Average", Options.Average);
	Options.FullScreen = get_int(nullptr, "FullScreen", Options.FullScreen);
	Options.PriorityAdj = get_int(nullptr, "Priority_Adjustment", Options.PriorityAdj);
	Options.Players = get_int(nullptr, "Players", Options.Players);
	Options.LeftFlipperKey = get_int(nullptr, "Left Flippper key", Options.LeftFlipperKey);
	Options.RightFlipperKey = get_int(nullptr, "Right Flipper key", Options.RightFlipperKey);
	Options.PlungerKey = get_int(nullptr, "Plunger key", Options.PlungerKey);
	Options.LeftTableBumpKey = get_int(nullptr, "Left Table Bump key", Options.LeftTableBumpKey);
	Options.RightTableBumpKey = get_int(nullptr, "Right Table Bump key", Options.RightTableBumpKey);
	Options.BottomTableBumpKey = get_int(nullptr, "Bottom Table Bump key", Options.BottomTableBumpKey);
	Options.UniformScaling = get_int(nullptr, "Uniform scaling", true);*/
	Sound::Enable(0, 7, Options.Sounds);

	update_resolution_menu();
}

void options::uninit()
{
	/*set_int(nullptr, "Sounds", Options.Sounds);
	set_int(nullptr, "Music", Options.Music);
	set_int(nullptr, "FullScreen", Options.FullScreen);
	set_int(nullptr, "Players", Options.Players);
	set_int(nullptr, "Left Flippper key", Options.LeftFlipperKey);
	set_int(nullptr, "Right Flipper key", Options.RightFlipperKey);
	set_int(nullptr, "Plunger key", Options.PlungerKey);
	set_int(nullptr, "Left Table Bump key", Options.LeftTableBumpKey);
	set_int(nullptr, "Right Table Bump key", Options.RightTableBumpKey);
	set_int(nullptr, "Bottom Table Bump key", Options.BottomTableBumpKey);
	set_int(nullptr, "Screen Resolution", Options.Resolution);
	set_int(nullptr, "Uniform scaling", Options.UniformScaling);*/
}

void options::path_init(LPCSTR regPath)
{
	char* buf = memory::allocate(strlen(regPath) + 1);
	OptionsRegPath = buf;
	if (buf)
		strncpy(buf,  regPath, strlen(regPath) + 1);
}

void options::path_uninit()
{
	if (OptionsRegPath)
		memory::free((void*)OptionsRegPath);
	OptionsRegPath = nullptr;
}

LPCSTR options::path(LPCSTR regPath)
{
	char* buf = OptionsRegPathCur;
	if (!OptionsRegPathCur)
	{
		buf = memory::allocate(2000);
		OptionsRegPathCur = buf;
		if (!buf)
			return OptionsRegPath;
	}
	strncpy(buf, OptionsRegPath, 2000);
	if (!regPath)
		return OptionsRegPathCur;
	strcat(OptionsRegPathCur, "\\");
	strcat(OptionsRegPathCur, regPath);
	return OptionsRegPathCur;
}

void options::path_free()
{
	if (OptionsRegPathCur)
		memory::free(OptionsRegPathCur);
	OptionsRegPathCur = nullptr;
}


int options::get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue)
{
	auto result = defaultValue;
	if (!OptionsRegPath)
		return result;

	auto regPath = path(optPath);
	path_free();
	return result;
}

void options::set_int(LPCSTR optPath, LPCSTR lpValueName, int data)
{
	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);	
	path_free();
}

void options::get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR dst, LPCSTR defaultValue, int iMaxLength)
{
	strncpy(dst, defaultValue, iMaxLength);
	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);	
	path_free();
}

void options::set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value)
{
	if (!OptionsRegPath)
		return;

	auto regPath = path(optPath);	
	path_free();
}


void options::toggle(uint32_t uIDCheckItem)
{
	int newValue;
	switch (uIDCheckItem)
	{
	case Menu1_Sounds:
		newValue = Options.Sounds == 0;
		Options.Sounds = Options.Sounds == 0;
		Sound::Enable(0, 7, newValue);
		return;
	case Menu1_Music:
		newValue = Options.Music == 0;
		Options.Music = Options.Music == 0;
		if (!newValue)
			midi::music_stop();
		else
			midi::play_pb_theme(0);
		return;
	case Menu1_Full_Screen:
		newValue = Options.FullScreen == 0;
		Options.FullScreen = Options.FullScreen == 0;
		fullscrn::set_screen_mode(newValue);
		return;
	case Menu1_1Player:
	case Menu1_2Players:
	case Menu1_3Players:
	case Menu1_4Players:
		Options.Players = uIDCheckItem - Menu1_1Player + 1;
		break;
	case Menu1_MaximumResolution:
	case Menu1_640x480:
	case Menu1_800x600:
	case Menu1_1024x768:
		{
			/*for (unsigned i = Menu1_MaximumResolution; i <= Menu1_1024x768; ++i)
				menu_check(i, i == uIDCheckItem);*/

			int newResolution = uIDCheckItem - Menu1_640x480;
			if (uIDCheckItem == Menu1_MaximumResolution)
			{
				Options.Resolution = -1;
				if (fullscrn::GetMaxResolution() != fullscrn::GetResolution())
					winmain::Restart();
			}
			else if (newResolution != fullscrn::GetResolution() && newResolution <= fullscrn::GetMaxResolution())
			{
				Options.Resolution = newResolution;
				winmain::Restart();
			}
			break;
		}
	case Menu1_WindowUniformScale:
		Options.UniformScaling ^= true;
		fullscrn::window_size_changed();
		pb::paint();
		break;
	default:
		break;
	}
}

void options::update_resolution_menu()
{
	auto maxResolution = fullscrn::get_max_supported_resolution();
	fullscrn::SetMaxResolution(maxResolution);
	const char* maxResText = pinball::get_rc_string(maxResolution + 2030, 0);
	/*if (MenuHandle)
		ModifyMenuA(MenuHandle, Menu1_MaximumResolution, 0, Menu1_MaximumResolution, maxResText);

	for (auto resIndex = 0; resIndex < 3; resIndex++)
	{
		menu_set(fullscrn::resolution_array[resIndex].ResolutionMenuId, fullscrn::GetMaxResolution() >= resIndex);
	}
	for (auto i = Menu1_MaximumResolution; i <= Menu1_1024x768; ++i)
	{
		menu_check(i, 0);
	}
	if (Options.Resolution >= 0)
		menu_check(fullscrn::resolution_array[fullscrn::GetResolution()].ResolutionMenuId, 1);
	else
		menu_check(Menu1_MaximumResolution, 1);*/
}

void options::init_resolution()
{
	Options.Resolution = get_int(nullptr, "Screen Resolution", -1);
	int maxRes = fullscrn::get_max_supported_resolution();
	if (Options.Resolution == -1 || maxRes <= Options.Resolution)
	{
		fullscrn::SetMaxResolution(maxRes);
		fullscrn::SetResolution(maxRes);
	}
	else
	{
		fullscrn::SetResolution(Options.Resolution);
	}
}

void options::keyboard()
{
	//DialogBoxParamA(nullptr, "KEYMAPPER", nullptr, KeyMapDlgProc, 0);
}
