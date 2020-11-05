#include "pch.h"
#include "pinball.h"
#include "memory.h"


int pinball::quickFlag = 0;
int pinball::render_background_bitmap = 0;
TTextBox* pinball::InfoTextBox;
TTextBox* pinball::MissTextBox;
char pinball::getRcBuffer[6 * 256];
int pinball::rc_string_slot = 0;
HINSTANCE pinball::hinst;
char pinball::WindowName[2]{};
char pinball::DatFileName[300]{};


char* pinball::get_rc_string(int uID, int a2)
{
	char* result = &getRcBuffer[256 * rc_string_slot];
	if (!LoadStringA(hinst, uID, &getRcBuffer[256 * rc_string_slot], 255))
		*result = 0;
	if (++rc_string_slot >= 6)
		rc_string_slot = 0;
	return result;
}
