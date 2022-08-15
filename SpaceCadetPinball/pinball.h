#pragma once

#include "translations.h"

class TTextBox;

class pinball
{
public:
	static int quickFlag;
	static TTextBox* InfoTextBox;
	static TTextBox* MissTextBox;
	static int RightShift;
	static int LeftShift;
	static std::string BasePath;

	static char* get_rc_string(translation_id_e uID);
	static int get_rc_int(translation_id_e uID, int* dst);
	static std::string make_path_name(const std::string& fileName);
private:
	static char getRcBuffer[256 * 6];
	static int rc_string_slot;
};
