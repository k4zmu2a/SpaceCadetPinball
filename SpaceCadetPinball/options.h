#pragma once

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

enum class InputTypes: unsigned
{
	None = 0,
	Keyboard = 1,
	Mouse = 2,
	GameController = 3,
};

struct GameInput
{
	InputTypes Type;
	int Value;

	bool operator==(const GameInput& other) const
	{
		return Type == other.Type && Value == other.Value;
	}
};

struct ControlsStruct
{
	GameInput LeftFlipper[3];
	GameInput RightFlipper[3];
	GameInput Plunger[3];
	GameInput LeftTableBump[3];
	GameInput RightTableBump[3];
	GameInput BottomTableBump[3];
};

struct optionsStruct
{
	ControlsStruct Key;
	ControlsStruct KeyDft;
	bool Sounds;
	bool Music;
	bool FullScreen;
	int Players;
	int Resolution;
	bool UniformScaling;
	bool LinearFiltering;
	int FramesPerSecond;
	int UpdatesPerSecond;
	bool ShowMenu;
	bool UncappedUpdatesPerSecond;
	int SoundChannels;
	bool HybridSleep;
	bool Prefer3DPBGameData;
	bool IntegerScaling;
	int SoundVolume;
	int MusicVolume;
	bool SoundStereo;
	bool DebugOverlay;
	bool DebugOverlayGrid;
	bool DebugOverlayAllEdges;
	bool DebugOverlayBallPosition;
	bool DebugOverlayBallEdges;
	bool DebugOverlayCollisionMask;
	bool DebugOverlaySprites;
};

struct ControlRef
{
	const char* Name;
	GameInput (&Option)[3];
};


class options
{
public:
	// Original does ~120 updates per second.
	static constexpr int MaxUps = 360, MaxFps = MaxUps, MinUps = 60, MinFps = MinUps,
	                     DefUps = 120, DefFps = 60;
	// Original uses 8 sound channels
	static constexpr int MaxSoundChannels = 32, MinSoundChannels = 1, DefSoundChannels = 8;
	static constexpr int MaxVolume = MIX_MAX_VOLUME, MinVolume = 0, DefVolume = MaxVolume;
	static optionsStruct Options;

	static void InitPrimary();
	static void InitSecondary();
	static void uninit();
	static int get_int(LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR lpValueName, int data);
	static std::string get_string(LPCSTR lpValueName, LPCSTR defaultValue);
	static void set_string(LPCSTR lpValueName, LPCSTR value);
	static float get_float(LPCSTR lpValueName, float defaultValue);
	static void set_float(LPCSTR lpValueName, float data);
	static void GetInput(const std::string& rowName, GameInput (&defaultValues)[3]);
	static void SetInput(const std::string& rowName, GameInput (&values)[3]);
	static void toggle(Menu1 uIDCheckItem);
	static void InputDown(GameInput input);
	static void ShowControlDialog();
	static void RenderControlDialog();
	static bool WaitingForInput() { return ControlWaitingForInput != nullptr; }
private:
	static std::map<std::string, std::string> settings;
	static ControlsStruct RebindControls;
	static bool ShowDialog;
	static const ControlRef Controls[6];
	static GameInput* ControlWaitingForInput;

	static void MyUserData_ReadLine(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
	static void* MyUserData_ReadOpen(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
	static void MyUserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);
	static const std::string& GetSetting(const std::string& key, const std::string& value);
	static void SetSetting(const std::string& key, const std::string& value);
};
