#include "pch.h"
#include "options.h"
#include "memory.h"

LPCSTR options::OptionsRegPath;
LPSTR options::OptionsRegPathCur;

void options::path_init(LPCSTR regPath)
{
	char* buf = memory::allocate(lstrlenA(regPath) + 1);
	OptionsRegPath = buf;
	if (buf)
		lstrcpyA(buf, regPath);
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
		buf = memory::allocate(0x7D0u);
		OptionsRegPathCur = buf;
		if (!buf)
			return OptionsRegPath;
	}
	lstrcpyA(buf, OptionsRegPath);
	if (!regPath)
		return OptionsRegPathCur;
	lstrcatA(OptionsRegPathCur, "\\");
	lstrcatA(OptionsRegPathCur, regPath);
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
	DWORD dwDisposition; // [esp+4h] [ebp-8h]	

	HKEY result = (HKEY)defaultValue, Data = (HKEY)defaultValue;
	if (!OptionsRegPath)
		return defaultValue;
	LPCSTR regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, &result, &dwDisposition))
	{
		optPath = (LPCSTR)4;
		RegQueryValueExA(result, lpValueName, nullptr, nullptr, (LPBYTE)&Data, (LPDWORD)&optPath);
		RegCloseKey(result);
	}
	path_free();
	return (int)Data;
}

void options::set_int(LPCSTR optPath, LPCSTR lpValueName, int data)
{
	DWORD dwDisposition; // [esp+4h] [ebp-4h]

	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&optPath,
		                     &dwDisposition))
		{
			RegSetValueExA((HKEY)optPath, lpValueName, 0, 4u, (const BYTE*)&data, 4u);
			RegCloseKey((HKEY)optPath);
		}
		path_free();
	}
}

void options::get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR lpString1, LPCSTR lpString2, int iMaxLength)
{
	const CHAR* v5 = (const CHAR*)iMaxLength;
	lstrcpynA(lpString1, lpString2, iMaxLength);
	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&iMaxLength,
		                     (LPDWORD)&optPath))
		{
			lpString2 = v5;
			RegQueryValueExA((HKEY)iMaxLength, lpValueName, nullptr, nullptr, (LPBYTE)lpString1, (LPDWORD)&lpString2);
			RegCloseKey((HKEY)iMaxLength);
		}
		path_free();
	}
}

void options::set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value)
{
	DWORD dwDisposition; // [esp+4h] [ebp-4h]

	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&optPath,
		                     &dwDisposition))
		{
			int v4 = lstrlenA(value);
			RegSetValueExA((HKEY)optPath, lpValueName, 0, 1u, (const BYTE*)value, v4 + 1);
			RegCloseKey((HKEY)optPath);
		}
		path_free();
	}
}
