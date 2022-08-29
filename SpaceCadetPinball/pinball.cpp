#include "pch.h"
#include "pinball.h"
#include "translations.h"
#include "winmain.h"

int LoadStringAlt(Msg uID, LPSTR lpBuffer, int cchBufferMax)
{
	const char* text = translations::GetTranslation(uID);

	strncpy(lpBuffer, text, cchBufferMax);
	return 1;
}

int pinball::quickFlag = 0;
TTextBox* pinball::InfoTextBox;
TTextBox* pinball::MissTextBox;
char pinball::getRcBuffer[6 * 256];
int pinball::rc_string_slot = 0;
int pinball::LeftShift = -1;
int pinball::RightShift = -1;
std::string pinball::BasePath;

char* pinball::get_rc_string(Msg uID)
{
	char* result = &getRcBuffer[256 * rc_string_slot];
	if (!LoadStringAlt(uID, &getRcBuffer[256 * rc_string_slot], 255))
		*result = 0;
	
	if (++rc_string_slot >= 6)
		rc_string_slot = 0;
	return result;
}

int pinball::get_rc_int(Msg uID, int* dst)
{
	char buffer[255];
	int result = LoadStringAlt(uID, buffer, 255);
	if (!result)
		return result;
	*dst = atoi(buffer);
	return 1;
}

std::string pinball::make_path_name(const std::string& fileName)
{
	return BasePath + fileName;
}
