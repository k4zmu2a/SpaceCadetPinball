#pragma once
#include "TTextBox.h"

struct winhelp_entry
{
	int ControlId;
	int ContextId;
};

class pinball
{
public:
	static int quickFlag;
	static TTextBox* InfoTextBox;
	static TTextBox* MissTextBox;
	static char WindowName[2];
	static int RightShift;
	static int LeftShift;

	static char* get_rc_string(int uID, int a2);
	static int get_rc_int(int uID, int* dst);
	static void FindShiftKeys();
	static HANDLE adjust_priority(int priority);
	static int make_path_name(LPSTR lpFilename, LPCSTR lpString2, int nSize = 0x12Cu);
private:
	static char getRcBuffer[256 * 6];
	static int rc_string_slot;
};
