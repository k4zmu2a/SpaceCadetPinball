#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pb.h"
#include "pinball.h"
#include "resource.h"
#include "Sound.h"
#include "winmain.h"

optionsStruct options::Options{};

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

std::map<std::string, std::string> options::settings{};


void options::init()
{
	auto imContext = ImGui::GetCurrentContext();
	ImGuiSettingsHandler ini_handler;
	ini_handler.TypeName = "Pinball";
	ini_handler.TypeHash = ImHashStr(ini_handler.TypeName);
	ini_handler.ReadOpenFn = MyUserData_ReadOpen;
	ini_handler.ReadLineFn = MyUserData_ReadLine;
	ini_handler.WriteAllFn = MyUserData_WriteAll;
	imContext->SettingsHandlers.push_back(ini_handler);

	// Settings are loaded from disk on the first frame
	if (!imContext->SettingsLoaded)
	{
		ImGui::NewFrame();
		ImGui::EndFrame();
	}

	Options.Sounds = 1;
	Options.Music = 0;
	Options.FullScreen = 0;
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
	Options.Sounds = get_int("Sounds", Options.Sounds);
	Options.Music = get_int("Music", Options.Music);
	Options.FullScreen = get_int("FullScreen", Options.FullScreen);
	Options.Players = get_int("Players", Options.Players);
	Options.LeftFlipperKey = get_int("Left Flipper key", Options.LeftFlipperKey);
	Options.RightFlipperKey = get_int("Right Flipper key", Options.RightFlipperKey);
	Options.PlungerKey = get_int("Plunger key", Options.PlungerKey);
	Options.LeftTableBumpKey = get_int("Left Table Bump key", Options.LeftTableBumpKey);
	Options.RightTableBumpKey = get_int("Right Table Bump key", Options.RightTableBumpKey);
	Options.BottomTableBumpKey = get_int("Bottom Table Bump key", Options.BottomTableBumpKey);
	Options.UniformScaling = get_int("Uniform scaling", Options.UniformScaling);
	Sound::Enable(0, 7, Options.Sounds);

	update_resolution_menu();
}

void options::uninit()
{
	set_int("Sounds", Options.Sounds);
	set_int("Music", Options.Music);
	set_int("FullScreen", Options.FullScreen);
	set_int("Players", Options.Players);
	set_int("Left Flipper key", Options.LeftFlipperKey);
	set_int("Right Flipper key", Options.RightFlipperKey);
	set_int("Plunger key", Options.PlungerKey);
	set_int("Left Table Bump key", Options.LeftTableBumpKey);
	set_int("Right Table Bump key", Options.RightTableBumpKey);
	set_int("Bottom Table Bump key", Options.BottomTableBumpKey);
	set_int("Screen Resolution", Options.Resolution);
	set_int("Uniform scaling", Options.UniformScaling);
}


int options::get_int(LPCSTR lpValueName, int defaultValue)
{
	auto value = GetSetting(lpValueName, std::to_string(defaultValue));
	return std::stoi(value);
}

void options::set_int(LPCSTR lpValueName, int data)
{
	SetSetting(lpValueName, std::to_string(data));
}

std::string options::get_string(LPCSTR lpValueName, LPCSTR defaultValue)
{
	return GetSetting(lpValueName, defaultValue);
}

void options::set_string(LPCSTR lpValueName, LPCSTR value)
{
	SetSetting(lpValueName, value);
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
	Options.Resolution = get_int("Screen Resolution", -1);
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

void options::MyUserData_ReadLine(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line)
{
	auto& keyValueStore = *static_cast<std::map<std::string, std::string>*>(entry);
	std::string keyValue = line;
	auto separatorPos = keyValue.find('=');
	if (separatorPos != std::string::npos)
	{
		auto key = keyValue.substr(0, separatorPos);
		auto value = keyValue.substr(separatorPos + 1, keyValue.size());
		keyValueStore[key] = value;
	}
}

void* options::MyUserData_ReadOpen(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name)
{
	// There is only one custom entry
	return strcmp(name, "Settings") == 0 ? &settings : nullptr;
}

void options::MyUserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
	buf->appendf("[%s][%s]\n", handler->TypeName, "Settings");
	for (const auto& setting : settings)
	{
		buf->appendf("%s=%s\n", setting.first.c_str(), setting.second.c_str());
	}
	buf->append("\n");
}

const std::string& options::GetSetting(const std::string& key, const std::string& value)
{
	auto setting = settings.find(key);
	if (setting == settings.end())
	{
		settings[key] = value;
		if (ImGui::GetCurrentContext())
			ImGui::MarkIniSettingsDirty();
		return value;
	}
	return setting->second;
}

void options::SetSetting(const std::string& key, const std::string& value)
{
	settings[key] = value;
	if (ImGui::GetCurrentContext())
		ImGui::MarkIniSettingsDirty();
}
