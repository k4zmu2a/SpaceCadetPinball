#pragma once

struct __declspec(align(4)) optionsStruct
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
	int LeftFlipperKey2;
	int RightFlipperKey2;
	int PlungerKey2;
	int LeftTableBumpKey2;
	int RightTableBumpKey2;
	int BottomTableBumpKey2;
};


class options
{
public:
	static void init(HMENU menuHandle);
	static void uninit();
	static void path_init(LPCSTR regPath);
	static void path_uninit();
	static int get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR optPath, LPCSTR lpValueName, int data);
	static void get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
	static void set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value);
	static void menu_check(UINT uIDCheckItem, int check);
	static void menu_set(UINT uIDEnableItem, int enable);
	static void toggle(UINT uIDCheckItem);
	static void keyboard();
	static INT_PTR _stdcall KeyMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static optionsStruct Options;
private:
	static LPCSTR OptionsRegPath;
	static LPSTR OptionsRegPathCur;
	static LPCSTR path(LPCSTR regPath);
	static void path_free();
	static HMENU MenuHandle;
};
