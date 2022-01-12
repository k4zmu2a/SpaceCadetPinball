#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "midi.h"
#include "render.h"
#include "Sound.h"
#include "winmain.h"

constexpr int options::MaxUps, options::MaxFps, options::MinUps, options::MinFps, options::DefUps, options::DefFps;
constexpr int options::MaxSoundChannels, options::MinSoundChannels, options::DefSoundChannels;
constexpr int options::MaxVolume, options::MinVolume, options::DefVolume;

optionsStruct options::Options{};
std::map<std::string, std::string> options::settings{};
ControlsStruct options::RebindControls{};
bool options::ShowDialog = false;
GameInput* options::ControlWaitingForInput = nullptr;
const ControlRef options::Controls[6]
{
	{"Left Flipper", RebindControls.LeftFlipper},
	{"Right Flipper", RebindControls.RightFlipper},
	{"Left Table Bump", RebindControls.LeftTableBump},
	{"Right Table Bump", RebindControls.RightTableBump},
	{"Bottom Table Bump", RebindControls.BottomTableBump},
	{"Plunger", RebindControls.Plunger},
};


void options::InitPrimary()
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

	Options.Key = Options.KeyDft =
	{
		{
			{InputTypes::Keyboard, SDLK_z},
			{InputTypes::Mouse, SDL_BUTTON_LEFT},
			{InputTypes::GameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
		},
		{
			{InputTypes::Keyboard, SDLK_SLASH},
			{InputTypes::Mouse, SDL_BUTTON_RIGHT},
			{InputTypes::GameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
		},
		{
			{InputTypes::Keyboard, SDLK_SPACE},
			{InputTypes::Mouse, SDL_BUTTON_MIDDLE},
			{InputTypes::GameController, SDL_CONTROLLER_BUTTON_A},
		},
		{
			{InputTypes::Keyboard, SDLK_x},
			{InputTypes::Mouse, SDL_BUTTON_X1},
			{InputTypes::GameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT},
		},
		{
			{InputTypes::Keyboard, SDLK_PERIOD},
			{InputTypes::Mouse, SDL_BUTTON_X2},
			{InputTypes::GameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
		},
		{
			{InputTypes::Keyboard, SDLK_UP},
			{InputTypes::Mouse, SDL_BUTTON_X2 + 1},
			{InputTypes::GameController, SDL_CONTROLLER_BUTTON_DPAD_UP},
		},
	};
	GetInput("Left Flipper key", Options.Key.LeftFlipper);
	GetInput("Right Flipper key", Options.Key.RightFlipper);
	GetInput("Plunger key", Options.Key.Plunger);
	GetInput("Left Table Bump key", Options.Key.LeftTableBump);
	GetInput("Right Table Bump key", Options.Key.RightTableBump);
	GetInput("Bottom Table Bump key", Options.Key.BottomTableBump);

	Options.Sounds = get_int("Sounds", true);
	Options.Music = get_int("Music", false);
	Options.FullScreen = get_int("FullScreen", false);
	Options.Players = get_int("Players", 1);
	Options.UniformScaling = get_int("Uniform scaling", true);
	ImGui::GetIO().FontGlobalScale = get_float("UI Scale", 1.0f);
	Options.Resolution = get_int("Screen Resolution", -1);
	Options.LinearFiltering = get_int("Linear Filtering", true);
	Options.FramesPerSecond = Clamp(get_int("Frames Per Second", DefFps), MinFps, MaxFps);
	Options.UpdatesPerSecond = Clamp(get_int("Updates Per Second", DefUps), MinUps, MaxUps);
	Options.UpdatesPerSecond = std::max(Options.UpdatesPerSecond, Options.FramesPerSecond);
	Options.ShowMenu = get_int("ShowMenu", true);
	Options.UncappedUpdatesPerSecond = get_int("Uncapped Updates Per Second", false);
	Options.SoundChannels = Clamp(get_int("Sound Channels", DefSoundChannels), MinSoundChannels, MaxSoundChannels);
	Options.HybridSleep = get_int("HybridSleep", false);
	Options.Prefer3DPBGameData = get_int("Prefer 3DPB Game Data", false);
	Options.IntegerScaling = get_int("Integer Scaling", false);
	Options.SoundVolume = Clamp(get_int("Sound Volume", DefVolume), MinVolume, MaxVolume);
	Options.MusicVolume = Clamp(get_int("Music Volume", DefVolume), MinVolume, MaxVolume);
}

void options::InitSecondary()
{
	winmain::UpdateFrameRate();

	auto maxRes = fullscrn::GetMaxResolution();
	if (Options.Resolution >= 0 && Options.Resolution > maxRes)
		Options.Resolution = maxRes;
	fullscrn::SetResolution(Options.Resolution == -1 ? maxRes : Options.Resolution);
}

void options::uninit()
{
	SetInput("Left Flipper key", Options.Key.LeftFlipper);
	SetInput("Right Flipper key", Options.Key.RightFlipper);
	SetInput("Plunger key", Options.Key.Plunger);
	SetInput("Left Table Bump key", Options.Key.LeftTableBump);
	SetInput("Right Table Bump key", Options.Key.RightTableBump);
	SetInput("Bottom Table Bump key", Options.Key.BottomTableBump);

	set_int("Sounds", Options.Sounds);
	set_int("Music", Options.Music);
	set_int("FullScreen", Options.FullScreen);
	set_int("Players", Options.Players);
	set_int("Screen Resolution", Options.Resolution);
	set_int("Uniform scaling", Options.UniformScaling);
	set_float("UI Scale", ImGui::GetIO().FontGlobalScale);
	set_int("Linear Filtering", Options.LinearFiltering);
	set_int("Frames Per Second", Options.FramesPerSecond);
	set_int("Updates Per Second", Options.UpdatesPerSecond);
	set_int("ShowMenu", Options.ShowMenu);
	set_int("Uncapped Updates Per Second", Options.UncappedUpdatesPerSecond);
	set_int("Sound Channels", Options.SoundChannels);
	set_int("HybridSleep", Options.HybridSleep);
	set_int("Prefer 3DPB Game Data", Options.Prefer3DPBGameData);
	set_int("Integer Scaling", Options.IntegerScaling);
	set_int("Sound Volume", Options.SoundVolume);
	set_int("Music Volume", Options.MusicVolume);
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

void options::GetInput(const std::string& rowName, GameInput (&defaultValues)[3])
{
	for (auto i = 0u; i <= 2; i++)
	{
		auto name = rowName + " " + std::to_string(i);
		auto inputType = static_cast<InputTypes>(get_int((name + " type").c_str(), -1));
		auto input = get_int((name + " input").c_str(), -1);
		if (inputType <= InputTypes::GameController && input != -1)
			defaultValues[i] = {inputType, input};
	}
}

void options::SetInput(const std::string& rowName, GameInput (&values)[3])
{
	for (auto i = 0u; i <= 2; i++)
	{
		auto input = values[i];
		auto name = rowName + " " + std::to_string(i);
		set_int((name + " type").c_str(), static_cast<int>(input.Type));
		set_int((name + " input").c_str(), input.Value);
	}
}

void options::toggle(Menu1 uIDCheckItem)
{
	switch (uIDCheckItem)
	{
	case Menu1::Sounds:
		Options.Sounds ^= true;
		Sound::Enable(Options.Sounds);
		return;
	case Menu1::Music:
		Options.Music ^= true;
		if (!Options.Music)
			midi::music_stop();
		else
			midi::play_pb_theme();
		return;
	case Menu1::Show_Menu:
		Options.ShowMenu = Options.ShowMenu == 0;
		fullscrn::window_size_changed();
		return;
	case Menu1::Full_Screen:
		Options.FullScreen ^= true;
		fullscrn::set_screen_mode(Options.FullScreen);
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
		render::recreate_screen_texture();
		break;
	case Menu1::Prefer3DPBGameData:
		Options.Prefer3DPBGameData ^= true;
		winmain::Restart();
		break;
	case Menu1::WindowIntegerScale:
		Options.IntegerScaling ^= true;
		fullscrn::window_size_changed();
		break;
	default:
		break;
	}
}

void options::InputDown(GameInput input)
{
	if (ControlWaitingForInput)
	{
		// Skip function keys, just in case.
		if (input.Type == InputTypes::Keyboard && input.Value >= SDLK_F1 && input.Value <= SDLK_F12)
			return;

		// Start is reserved for pause
		if (input.Type == InputTypes::GameController && input.Value == SDL_CONTROLLER_BUTTON_START)
			return;

		*ControlWaitingForInput = input;
		ControlWaitingForInput = nullptr;
	}
}

void options::ShowControlDialog()
{
	if (!ShowDialog)
	{
		ControlWaitingForInput = nullptr;
		RebindControls = Options.Key;
		ShowDialog = true;
	}
}

void options::RenderControlDialog()
{
	static const char* mouseButtons[]
	{
		nullptr,
		"Mouse Left",
		"Mouse Middle",
		"Mouse Right",
		"Mouse X1",
		"Mouse X2",
	};

	if (!ShowDialog)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2{550, 450});
	if (ImGui::Begin("3D Pinball: Player Controls", &ShowDialog))
	{
		ImGui::TextUnformatted("Instructions");
		ImGui::Separator();

		ImGui::TextWrapped(
			"To change game controls, click the control button, press the new key, and then choose OK.");
		ImGui::TextWrapped(
			"To restore 3D Pinball to its original settings, choose Default, and then choose OK.");
		ImGui::Spacing();

		ImGui::TextUnformatted("Control Options");

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{5, 10});
		if (ImGui::BeginTable("Controls", 4, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
		{
			ImGui::TableSetupColumn("Control");
			ImGui::TableSetupColumn("Binding 1");
			ImGui::TableSetupColumn("Binding 2");
			ImGui::TableSetupColumn("Binding 3");
			ImGui::TableHeadersRow();

			int index = 0;
			for (auto& row : Controls)
			{
				ImGui::TableNextColumn();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.5, 0, 0, 1});
				if (ImGui::Button(row.Name))
				{
					for (auto i = 0u; i <= 2; i++)
						row.Option[i] = {};
				}
				ImGui::PopStyleColor(1);

				for (auto i = 0u; i <= 2; i++)
				{
					auto& ctrl = row.Option[i];
					ImGui::TableNextColumn();
					if (ControlWaitingForInput == &ctrl)
					{
						ImGui::Button("Press the key", ImVec2(-1, 0));
					}
					else
					{
						std::string tmp;
						const char* keyName;
						switch (ctrl.Type)
						{
						case InputTypes::Keyboard:
							keyName = SDL_GetKeyName(ctrl.Value);
							break;
						case InputTypes::Mouse:
							if (ctrl.Value >= SDL_BUTTON_LEFT && ctrl.Value <= SDL_BUTTON_X2)
								keyName = mouseButtons[ctrl.Value];
							else
								keyName = (tmp += "Mouse " + std::to_string(ctrl.Value)).c_str();
							break;
						case InputTypes::GameController:
							keyName = SDL_GameControllerGetStringForButton(
								static_cast<SDL_GameControllerButton>(ctrl.Value));
							break;
						case InputTypes::None:
						default:
							keyName = "Unused";
						}
						if (!keyName || !keyName[0])
							keyName = "Unknown key";
						if (ImGui::Button((std::string{keyName} + "##" + std::to_string(index++)).c_str(),
						                  ImVec2(-1, 0)))
						{
							ControlWaitingForInput = &ctrl;
						}
					}
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
			ControlWaitingForInput = nullptr;
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();

	if (!ShowDialog)
		ControlWaitingForInput = nullptr;
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
