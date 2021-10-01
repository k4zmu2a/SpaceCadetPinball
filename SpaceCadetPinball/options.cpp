#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "pb.h"
#include "Sound.h"
#include "winmain.h"

constexpr int options::MaxUps, options::MaxFps, options::MinUps, options::MinFps, options::DefUps, options::DefFps;

optionsStruct options::Options{};
std::map<std::string, std::string> options::settings{};
ControlsStruct options::RebindControls{};
bool options::ShowDialog = false;
const ControlRef* options::ControlWaitingForKey = nullptr;
const ControlRef options::Controls[6]
{
	{"Left Flipper", RebindControls.LeftFlipper},
	{"Right Flipper", RebindControls.RightFlipper},
	{"Left Table Bump", RebindControls.LeftTableBump},
	{"Right Table Bump", RebindControls.RightTableBump},
	{"Bottom Table Bump", RebindControls.BottomTableBump},
	{"Plunger", RebindControls.Plunger},
};


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
	Options.KeyDft.LeftFlipper = SDLK_z;
	Options.KeyDft.RightFlipper = SDLK_SLASH;
	Options.KeyDft.Plunger = SDLK_SPACE;
	Options.KeyDft.LeftTableBump = SDLK_x;
	Options.KeyDft.RightTableBump = SDLK_PERIOD;
	Options.KeyDft.BottomTableBump = SDLK_UP;
	Options.Key = Options.KeyDft;
	Options.Players = 1;

	Options.UniformScaling = true;
	Options.Sounds = get_int("Sounds", Options.Sounds);
	Options.Music = get_int("Music", Options.Music);
	Options.FullScreen = get_int("FullScreen", Options.FullScreen);
	Options.Players = get_int("Players", Options.Players);
	Options.Key.LeftFlipper = get_int("Left Flipper key", Options.Key.LeftFlipper);
	Options.Key.RightFlipper = get_int("Right Flipper key", Options.Key.RightFlipper);
	Options.Key.Plunger = get_int("Plunger key", Options.Key.Plunger);
	Options.Key.LeftTableBump = get_int("Left Table Bump key", Options.Key.LeftTableBump);
	Options.Key.RightTableBump = get_int("Right Table Bump key", Options.Key.RightTableBump);
	Options.Key.BottomTableBump = get_int("Bottom Table Bump key", Options.Key.BottomTableBump);
	Options.UniformScaling = get_int("Uniform scaling", Options.UniformScaling);
	ImGui::GetIO().FontGlobalScale = get_float("UI Scale", 1.0f);
	Options.Resolution = get_int("Screen Resolution", -1);
	Options.LinearFiltering = get_int("Linear Filtering", true);
	Options.FramesPerSecond = std::min(MaxFps, std::max(MinUps, get_int("Frames Per Second", DefFps)));
	Options.UpdatesPerSecond = std::min(MaxUps, std::max(MinUps, get_int("Updates Per Second", DefUps)));
	Options.UpdatesPerSecond = std::max(Options.UpdatesPerSecond, Options.FramesPerSecond);
	Options.ShowMenu = get_int("ShowMenu", true);

	winmain::UpdateFrameRate();

	Sound::Enable(0, 7, Options.Sounds);

	auto maxRes = fullscrn::GetMaxResolution();
	if (Options.Resolution >= 0 && Options.Resolution > maxRes)
		Options.Resolution = maxRes;
	fullscrn::SetResolution(Options.Resolution == -1 ? maxRes : Options.Resolution);
}

void options::uninit()
{
	set_int("Sounds", Options.Sounds);
	set_int("Music", Options.Music);
	set_int("FullScreen", Options.FullScreen);
	set_int("Players", Options.Players);
	set_int("Left Flipper key", Options.Key.LeftFlipper);
	set_int("Right Flipper key", Options.Key.RightFlipper);
	set_int("Plunger key", Options.Key.Plunger);
	set_int("Left Table Bump key", Options.Key.LeftTableBump);
	set_int("Right Table Bump key", Options.Key.RightTableBump);
	set_int("Bottom Table Bump key", Options.Key.BottomTableBump);
	set_int("Screen Resolution", Options.Resolution);
	set_int("Uniform scaling", Options.UniformScaling);
	set_float("UI Scale", ImGui::GetIO().FontGlobalScale);
	set_int("Linear Filtering", Options.LinearFiltering);
	set_int("Frames Per Second", Options.FramesPerSecond);
	set_int("Updates Per Second", Options.UpdatesPerSecond);
	set_int("ShowMenu", Options.ShowMenu);
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

float options::get_float(LPCSTR lpValueName, float defaultValue)
{
	auto value = GetSetting(lpValueName, std::to_string(defaultValue));
	return std::stof(value);
}

void options::set_float(LPCSTR lpValueName, float data)
{
	SetSetting(lpValueName, std::to_string(data));
}


void options::toggle(Menu1 uIDCheckItem)
{
	int newValue;
	switch (uIDCheckItem)
	{
	case Menu1::Sounds:
		newValue = Options.Sounds == 0;
		Options.Sounds = Options.Sounds == 0;
		Sound::Enable(0, 7, newValue);
		return;
	case Menu1::Music:
		newValue = Options.Music == 0;
		Options.Music = Options.Music == 0;
		if (!newValue)
			midi::music_stop();
		else
			midi::play_pb_theme();
		return;
	case Menu1::Show_Menu:
		Options.ShowMenu = Options.ShowMenu == 0;
		return;
	case Menu1::Full_Screen:
		newValue = Options.FullScreen == 0;
		Options.FullScreen = Options.FullScreen == 0;
		fullscrn::set_screen_mode(newValue);
		return;
	case Menu1::OnePlayer:
	case Menu1::TwoPlayers:
	case Menu1::ThreePlayers:
	case Menu1::FourPlayers:
		Options.Players = static_cast<int>(uIDCheckItem) - static_cast<int>(Menu1::OnePlayer) + 1;
		break;
	case Menu1::MaximumResolution:
	case Menu1::R640x480:
	case Menu1::R800x600:
	case Menu1::R1024x768:
		{
			auto restart = false;
			int newResolution = static_cast<int>(uIDCheckItem) - static_cast<int>(Menu1::R640x480);
			if (uIDCheckItem == Menu1::MaximumResolution)
			{
				restart = fullscrn::GetResolution() != fullscrn::GetMaxResolution();
				Options.Resolution = -1;
			}
			else if (newResolution <= fullscrn::GetMaxResolution())
			{
				restart = newResolution != (Options.Resolution == -1
					                            ? fullscrn::GetMaxResolution()
					                            : fullscrn::GetResolution());
				Options.Resolution = newResolution;
			}

			if (restart)
				winmain::Restart();
			break;
		}
	case Menu1::WindowUniformScale:
		Options.UniformScaling ^= true;
		fullscrn::window_size_changed();
		break;
	case Menu1::WindowLinearFilter:
		Options.LinearFiltering ^= true;
		winmain::Restart();
		break;
	default:
		break;
	}
}

void options::KeyDown(int key)
{
	if (ControlWaitingForKey)
	{
		// Skip function keys, just in case.
		if (key < SDLK_F1 || key > SDLK_F12)
		{
			ControlWaitingForKey->Option = key;
			ControlWaitingForKey = nullptr;
		}
	}
}

void options::ShowControlDialog()
{
	if (!ShowDialog)
	{
		ControlWaitingForKey = nullptr;
		RebindControls = Options.Key;
		ShowDialog = true;
	}
}

void options::RenderControlDialog()
{
	if (!ShowDialog)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2{500, 400});
	if (ImGui::Begin("3D Pinball: Player Controls", &ShowDialog))
	{
		ImGui::TextUnformatted("Instructions");
		ImGui::Separator();

		ImGui::TextWrapped(
			"To change game controls, click the control button, press the new key, and then choose OK.");
		ImGui::TextWrapped(
			"To restore 3D Pinball to its original settings, choose Default, and then choose OK.");
		ImGui::TextWrapped("Original warns against binding the same key to multiple controls, but does not forbid it.");
		ImGui::Spacing();

		ImGui::TextUnformatted("Control Options");
		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{5, 10});
		if (ImGui::BeginTable("Controls", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
		{
			for (auto& ctrl : Controls)
			{
				ImGui::TableNextColumn();
				if (ImGui::BeginTable("Control", 2, ImGuiTableFlags_NoSavedSettings))
				{
					ImGui::TableNextColumn();
					ImGui::TextWrapped("%s", ctrl.Name);

					ImGui::TableNextColumn();
					if (ControlWaitingForKey == &ctrl)
					{
						ImGui::Button("Press the key", ImVec2(-1, 0));
					}
					else
					{
						auto keyName = SDL_GetKeyName(ctrl.Option);
						if (!keyName[0])
							keyName = "Unknown key";
						if (ImGui::Button(keyName, ImVec2(-1, 0)))
						{
							ControlWaitingForKey = &ctrl;
						}
					}
					ImGui::EndTable();
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
		ImGui::Spacing();

		if (ImGui::Button("OK"))
		{
			Options.Key = RebindControls;
			ShowDialog = false;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ShowDialog = false;
		}

		ImGui::SameLine();
		if (ImGui::Button("Default"))
		{
			RebindControls = Options.KeyDft;
			ControlWaitingForKey = nullptr;
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();

	if (!ShowDialog)
		ControlWaitingForKey = nullptr;
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
