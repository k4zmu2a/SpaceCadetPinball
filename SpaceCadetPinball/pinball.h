#pragma once
#include "TTextBox.h"

class pinball
{
public:
	static int quickFlag;
	static int render_background_bitmap;
	static TTextBox* InfoTextBox;
	static TTextBox* MissTextBox;
	static HINSTANCE hinst;
	static char WindowName[2];
	static char DatFileName[300];
	static char* get_rc_string(int uID, int a2);
private:
	static char getRcBuffer[256 * 6];
	static int rc_string_slot;
};
