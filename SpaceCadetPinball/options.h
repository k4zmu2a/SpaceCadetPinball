#pragma once

enum class Msg : int;

enum class Menu1:int
{
	New_Game = 101,
	About_Pinball = 102,
	High_Scores = 103,
	Exit = 105,
	Sounds = 201,
	Music = 202,
	SoundStereo = 203,
	Help_Topics = 301,
	Launch_Ball = 401,
	Pause_Resume_Game = 402,
	Full_Screen = 403,
	Demo = 404,
	Select_Table = 405,
	Player_Controls = 406,
	OnePlayer = 408,
	TwoPlayers = 409,
	ThreePlayers = 410,
	FourPlayers = 411,
	Show_Menu = 412,
	MaximumResolution = 500,
	R640x480 = 501,
	R800x600 = 502,
	R1024x768 = 503,
	WindowUniformScale = 600,
	WindowLinearFilter = 601,
	WindowIntegerScale = 602,
	Prefer3DPBGameData = 700,
};

enum class InputTypes
{
	None = 0,
	Keyboard,
	Mouse,
	GameController,
};

struct GameInput
{
	InputTypes Type;
	int Value;

	bool operator==(const GameInput& other) const
	{
		return Type == other.Type && Value == other.Value;
	}

	std::string GetInputDescription() const;
};

enum class GameBindings
{
	Min = 0,
	LeftFlipper = 0,
	RightFlipper,
	Plunger,
	LeftTableBump,
	RightTableBump,
	BottomTableBump,
	Max
};

inline GameBindings& operator++(GameBindings& value, int)
{
	return value = static_cast<GameBindings>(static_cast<int>(value) + 1);
}

constexpr int operator~(const GameBindings& value)
{
	return static_cast<int>(value);
}

class options
{
public:
	// Original does ~120 updates per second.
	static constexpr int MaxUps = 360, MaxFps = MaxUps, MinUps = 60, MinFps = MinUps,
	                     DefUps = 120, DefFps = 60;
	// Original uses 8 sound channels
	static constexpr int MaxSoundChannels = 32, MinSoundChannels = 1, DefSoundChannels = 8;
	static constexpr int MaxVolume = MIX_MAX_VOLUME, MinVolume = 0, DefVolume = MaxVolume;
	static struct optionsStruct Options;
	static std::vector<struct OptionBase*> AllOptions;

	static void InitPrimary();
	static void InitSecondary();
	static void uninit();
	static const std::string& GetSetting(const std::string& key, const std::string& defaultValue);
	static void SetSetting(const std::string& key, const std::string& value);
	static int get_int(LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR lpValueName, int data);
	static float get_float(LPCSTR lpValueName, float defaultValue);
	static void set_float(LPCSTR lpValueName, float data);
	static void GetInput(const std::string& rowName, GameInput (&values)[3]);
	static void SetInput(const std::string& rowName, GameInput (&values)[3]);
	static void toggle(Menu1 uIDCheckItem);
	static void InputDown(GameInput input);
	static void ShowControlDialog();
	static void RenderControlDialog();
	static bool WaitingForInput() { return ControlWaitingForInput; }
	static std::vector<GameBindings> MapGameInput(GameInput key);
private:
	static std::unordered_map<std::string, std::string> settings;
	static bool ShowDialog;
	static GameInput* ControlWaitingForInput;

	static void MyUserData_ReadLine(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
	static void* MyUserData_ReadOpen(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
	static void MyUserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);
};


struct OptionBase
{
	LPCSTR Name;

	OptionBase(LPCSTR name);
	virtual ~OptionBase();
	virtual void Load() = 0;
	virtual void Save() const = 0;
	virtual void Reset() = 0;
};

template <typename T>
struct OptionBaseT : OptionBase
{
	const T DefaultValue;
	T V;

	OptionBaseT(LPCSTR name, T defaultValue) : OptionBase(name), DefaultValue(std::move(defaultValue)), V()
	{
	}

	void Reset() override { V = DefaultValue; }
	operator T&() { return V; }

	OptionBaseT& operator=(const T& v)
	{
		V = v;
		return *this;
	}
};

struct IntOption : OptionBaseT<int>
{
	IntOption(LPCSTR name, int defaultValue) : OptionBaseT(name, defaultValue)
	{
	}

	void Load() override { V = options::get_int(Name, DefaultValue); }
	void Save() const override { options::set_int(Name, V); }
	using OptionBaseT::operator=;
};

struct StringOption : OptionBaseT<std::string>
{
	StringOption(LPCSTR name, std::string defaultValue) : OptionBaseT(name, std::move(defaultValue))
	{
	}

	void Load() override { V = options::GetSetting(Name, DefaultValue); }
	void Save() const override { options::SetSetting(Name, V); }
};

struct FloatOption : OptionBaseT<float>
{
	FloatOption(LPCSTR name, float defaultValue) : OptionBaseT(name, defaultValue)
	{
	}

	void Load() override { V = options::get_float(Name, DefaultValue); }
	void Save() const override { options::set_float(Name, V); }
};

struct BoolOption : OptionBaseT<bool>
{
	BoolOption(LPCSTR name, bool defaultValue) : OptionBaseT(name, defaultValue)
	{
	}

	void Load() override { V = options::get_int(Name, DefaultValue); }
	void Save() const override { options::set_int(Name, V); }
	using OptionBaseT::operator=;
};

struct ControlOption : OptionBase
{
	GameInput Defaults[3];
	GameInput Inputs[3];

	ControlOption(LPCSTR name, GameInput defaultKeyboard, GameInput defaultMouse, GameInput defaultController) :
		OptionBase(name),
		Defaults{defaultKeyboard, defaultMouse, defaultController},
		Inputs{
			{InputTypes::Keyboard, -1},
			{InputTypes::Mouse, -1},
			{InputTypes::GameController, -1}
		}
	{
	}

	void Load() override
	{
		for (auto i = 0u; i <= 2; i++)
		{
			auto name = std::string{ Name } + " " + std::to_string(i);
			Inputs[i].Type = static_cast<InputTypes>(options::get_int((name + " type").c_str(),
				static_cast<int>(Defaults[i].Type)));
			Inputs[i].Value = options::get_int((name + " input").c_str(), Defaults[i].Value);
		}
	}

	void Save() const override
	{
		for (auto i = 0u; i <= 2; i++)
		{
			auto name = std::string{ Name } + " " + std::to_string(i);
			options::set_int((name + " type").c_str(), static_cast<int>(Inputs[i].Type));
			options::set_int((name + " input").c_str(), Inputs[i].Value);
		}
	}


	void Reset() override
	{
		std::copy(std::begin(Defaults), std::end(Defaults), std::begin(Inputs));
	}
};

struct optionsStruct
{
	ControlOption Key[~GameBindings::Max];
	BoolOption Sounds;
	BoolOption Music;
	BoolOption FullScreen;
	IntOption Players;
	IntOption Resolution;
	FloatOption UIScale;
	BoolOption UniformScaling;
	BoolOption LinearFiltering;
	IntOption FramesPerSecond;
	IntOption UpdatesPerSecond;
	BoolOption ShowMenu;
	BoolOption UncappedUpdatesPerSecond;
	IntOption SoundChannels;
	BoolOption HybridSleep;
	BoolOption Prefer3DPBGameData;
	BoolOption IntegerScaling;
	IntOption SoundVolume;
	IntOption MusicVolume;
	BoolOption SoundStereo;
	BoolOption DebugOverlay;
	BoolOption DebugOverlayGrid;
	BoolOption DebugOverlayAllEdges;
	BoolOption DebugOverlayBallPosition;
	BoolOption DebugOverlayBallEdges;
	BoolOption DebugOverlayCollisionMask;
	BoolOption DebugOverlaySprites;
	BoolOption DebugOverlaySounds;
	BoolOption DebugOverlayBallDepthGrid;
	BoolOption DebugOverlayAabb;
	StringOption FontFileName;
	StringOption Language;
};
