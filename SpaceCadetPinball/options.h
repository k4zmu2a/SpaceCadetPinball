#pragma once
#include "pinball.h"
#include "resource.h"

enum class Languages
{
	English = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
	Russian = MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA),
	TraditionalChinese = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL),
	SimplifiedChinese = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
	Polish = MAKELANGID(LANG_POLISH, SUBLANG_POLISH_POLAND),
	German = MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN),
	Japanese = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),
	French = MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH),
	Italian = MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN),
	Hungarian = MAKELANGID(LANG_HUNGARIAN, SUBLANG_HUNGARIAN_HUNGARY),
	Portuguese = MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE),
	BrazilianPortuguese = MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN),
	Czech = MAKELANGID(LANG_CZECH, SUBLANG_CZECH_CZECH_REPUBLIC),
	Danish = MAKELANGID(LANG_DANISH, SUBLANG_DANISH_DENMARK),
	Finnish = MAKELANGID(LANG_FINNISH, SUBLANG_FINNISH_FINLAND),
	Hebrew = MAKELANGID(LANG_HEBREW, SUBLANG_HEBREW_ISRAEL),
	Arabic = MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA),
	Greek = MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE),
	Spanish = MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN),
	Korean = MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN),
	Dutch = MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH),
	Norwegian = MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL),
	Swedish = MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH),
	Turkish = MAKELANGID(LANG_TURKISH, SUBLANG_TURKISH_TURKEY),
};

struct LanguageMenuEntry
{
	LPCWSTR Name;
	Languages Language;
};

struct optionsStruct
{
	int Sounds;
	int Music;
	int Average;
	int FullScreen;
	int PriorityAdj;
	int Players;
	int LeftFlipperKey;
	int RightFlipperKey;
	int PlungerKey;
	int LeftTableBumpKey;
	int RightTableBumpKey;
	int BottomTableBumpKey;
	int LeftFlipperKeyDft;
	int RightFlipperKeyDft;
	int PlungerKeyDft;
	int LeftTableBumpKeyDft;
	int RightTableBumpKeyDft;
	int BottomTableBumpKeyDft;
	int Resolution;
	bool UniformScaling;
	bool AlternativeRender;
	int TargetUps;
	Languages Language;
};


class options
{
public:
	static void ReadOptions();
	static void init(HMENU menuHandle);
	static void uninit();
	static void path_init(LPCSTR regPath);
	static void path_uninit();
	static int get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR optPath, LPCSTR lpValueName, int data);
	static void get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR dst, LPCSTR defaultValue, int iMaxLength);
	static void set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value);
	static void get_string(LPCSTR optPath, LPCWSTR lpValueName, LPWSTR dst, LPCWSTR defaultValue, int iMaxLength);
	static void set_string(LPCSTR optPath, LPCWSTR lpValueName, LPCWSTR value);
	static void menu_check(UINT uIDCheckItem, int check);
	static void menu_set(UINT uIDEnableItem, int enable);
	static void toggle(UINT uIDCheckItem);
	static void update_resolution_menu();
	static void init_resolution();

	static void keyboard();
	static INT_PTR _stdcall KeyMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static LPWSTR get_vk_key_name(uint16_t vk, LPWSTR keyName);

	static optionsStruct Options;
private:
	static LPCSTR OptionsRegPath;
	static LPSTR OptionsRegPathCur;
	static LPCSTR path(LPCSTR regPath);
	static void path_free();
	static HMENU MenuHandle;
	static winhelp_entry keymap_help[18];
	static short vk_list[28];
	static LanguageMenuEntry LanguageMenu[Menu1_LanguageMax - Menu1_Language];
};
