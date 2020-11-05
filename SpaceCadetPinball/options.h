#pragma once
class options
{
public:
	static void path_init(LPCSTR regPath);
	static void path_uninit();
	static int get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR optPath, LPCSTR lpValueName, int data);
	static void get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
	static void set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value);
private:
	static LPCSTR OptionsRegPath;
	static LPSTR OptionsRegPathCur;
	static LPCSTR path(LPCSTR regPath);
	static void path_free();
};
