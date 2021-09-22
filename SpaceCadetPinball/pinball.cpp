#include "pch.h"
#include "pinball.h"
#include "memory.h"
#include "winmain.h"

// Todo: load translations from file
std::map<uint32_t, LPCSTR> rc_strings
{
	{0, "Replay Awarded"},
	{1, "Ball Locked"},
	{2, "Center Post\n%ld"},
	{3, "Bonus Awarded\n%ld"},
	{4, "Bonus Activated"},
	{5, "Weapons Upgraded"},
	{6, "Engine Upgraded"},
	{7, "Bonus up 1 Million"},
	{8, "Extra Ball Available\n%ld"},
	{9, "Extra Ball"},
	{10, "Reflex Shot Award\n%ld"},
	{11, "Final Battle Won"},
	{12, "Hyperspace Bonus\n%ld"},
	{13, "Hyperspace Bonus Available"},
	{14, "Jackpot Awarded\n%ld"},
	{15, "Jackpot Activated"},
	{16, "Multiball"},
	{17, "Ramp Bonus Awarded"},
	{18, "Light Added"},
	{19, "Ramp Bonus On"},
	{20, "Light Reset Off"},
	{21, "Skill Shot\n%ld"},
	{22, "Game Paused\nF3 to Resume"},
	{23, "Continue Play"},
	{24, "F2 Starts New Game"},
	{25, "Careful..."},
	{26, "Player 1"},
	{27, "Player 2"},
	{28, "Player 3"},
	{29, "Player 4"},
	{30, "Demo\nPlayer 1"},
	{31, "Demo\nPlayer 2"},
	{32, "Demo\nPlayer 3"},
	{33, "Demo\nPlayer 4"},
	{34, "Game Over"},
	{35, "TILT!"},
	{36, "This program requires an 80386 or later CPU."},
	{37, "80386 Required"},
	{38, "3D Pinball for Windows - Space Cadet"},
	{
		39,
		"One or more of the player controls is set to the same key,\nfor best performance use unique keys for each control."
	},
	{40, "Clear High Scores?"},
	{41, "Confirm"},
	{42, "WAVEMIX.INF is missing - it must be in the pinball directory!"},
	{43, "Warning:"},
	{44, "Ship Re-Fueled"},
	{45, "Gravity Well"},
	{46, "Time Warp Forward"},
	{47, "Time Warp Backward"},
	{48, "Maelstrom!"},
	{49, "Wormhole"},
	{50, "Awaiting Deployment"},
	{51, "Flags Upgraded"},
	{52, "Bonus Hold"},
	{53, "Level One Commation"},
	{54, "Level Two Commation"},
	{55, "Level Three Commation"},
	{56, "Field Multiplier 2x"},
	{57, "Field Multiplier 3x"},
	{58, "Field Multiplier 5x"},
	{59, "Field Multiplier 10x"},
	{60, "Target Practice"},
	{61, "Launch Training"},
	{62, "Re-Entry Training"},
	{63, "Science"},
	{64, "Stray Comet"},
	{65, "Black Hole"},
	{66, "Space Radiation"},
	{67, "Bug Hunt"},
	{68, "Alien Menace"},
	{69, "Rescue"},
	{70, "Satellite Retrieval"},
	{71, "Recon"},
	{72, "Doomsday Machine"},
	{73, "Cosmic Plague"},
	{74, "Secret"},
	{75, "Time Warp"},
	{76, "Maelstrom"},
	{77, "Mission Accepted\n%ld"},
	{78, "Mission Completed\n%ld"},
	{79, "%s Mission Selected"},
	{80, "Black Hole\n%ld"},
	{81, "Gravity Normalized\n%ld"},
	{82, "Gravity Well\n%ld"},
	{83, "Promotion to %s"},
	{84, "Cadet"},
	{85, "Ensign"},
	{86, "Lieutenant"},
	{87, "Captain"},
	{88, "Lt Commander"},
	{89, "Commander"},
	{90, "Commodore"},
	{91, "Admiral"},
	{92, "Fleet Admiral"},
	{93, "Wormhole Opened"},
	{94, "Crash Bonus\n%ld"},
	{95, "Replay Ball"},
	{96, "Re-Deploy"},
	{97, "Player 1 Shoot Again"},
	{98, "Player 2 Shoot Again"},
	{99, "Player 3 Shoot Again"},
	{100, "Player 4 Shoot Again"},
	{
		101,
		"This 3D Pinball Table was created for Microsoft by Maxis.\nFor more information call (800)-336-2947\n(US and Canadian customers only).\nCopyright (c) 1995 Maxis."
	},
	{102, "3D Pinball Table created for Microsoft by Maxis. Copyright (c) 1995 Maxis."},
	{103, "About 3D Pinball"},
	{104, "Hit Mission Targets To Select Mission"},
	{105, "Re-Fuel Ship"},
	{106, "Launch Ramp To Accept %s Mission"},
	{107, "Attack Bumpers Hits\nLeft: %d"},
	{108, "Target Training Passed"},
	{109, "Mission Aborted"},
	{110, "Launches Left: %d"},
	{111, "Launch Training Passed"},
	{112, "Re-Entries Left: %d"},
	{113, "Re-Entry Training Passed"},
	{114, "Drop Targets\nLeft: %d"},
	{115, "Science Mission Completed"},
	{116, "Warning -- Low Fuel"},
	{117, "Fill Right Hazard Banks"},
	{118, "Hyperspace Launch"},
	{119, "Comet Destroyed"},
	{120, "Enter Wormhole"},
	{121, "Radiation Eliminated"},
	{122, "Upgrade Launch Bumpers"},
	{123, "Enter Black Hole"},
	{124, "Black Hole Eliminated"},
	{125, "Targets\nLeft: %d"},
	{126, "Xenomorphs Destroyed"},
	{127, "Upgrade Flags"},
	{128, "Hyperspace Launch"},
	{129, "Survivors Rescued"},
	{130, "Aliens Repelled"},
	{131, "Hit Fuel Targets"},
	{132, "Remote Attack Bumper Hits\nLeft: %d"},
	{133, "Satellite Repaired"},
	{134, "Lane Passes\nLeft: %d"},
	{135, "Shoot Ball Up Fuel Chute"},
	{136, "Survey Complete"},
	{137, "Out Lane Passes\nLeft: %d"},
	{138, "Doomsday Machine Destroyed"},
	{139, "Roll Flags: %d"},
	{140, "Hit Space Warp Rollover"},
	{141, "Plague Eliminated"},
	{142, "Hit Yellow Wormhole"},
	{143, "Hit Red Wormhole"},
	{144, "Hit Green Wormhole"},
	{145, "Plans Recovered"},
	{146, "Rebound Hits\nLeft: %d"},
	{147, "Hit Hyperspace Chute or Launch Ramp"},
	{148, "Drop Target Hits\nLeft: %d"},
	{149, "Spot Target Hits\nLeft: %d"},
	{150, "Lanes Passes\nLeft: %d"},
	{151, "Shoot Ball Up Fuel Chute"},
	{152, "Hit Launch Ramp"},
	{153, "Hit Flags"},
	{154, "Hit Worm Hole"},
	{155, "Hyperspace Chute to  Maelstrom"},
	{156, "pinball.mid"},
	{158, "1 UseBitmapFont"},
	{159, "90 Left Flipper Key"},
	{160, "191 Right Flipper Key"},
	{161, "32 Plunger Key"},
	{162, "88 Bump Left Key"},
	{163, "190 Bump Right Key"},
	{164, "38 Bump Bottom Key"},
	{165, "Software\\Microsoft\\Plus!\\Pinball"},
	{166, "SpaceCadet"},
	{167, "1c7c22a0-9576-11ce-bf80-444553540000"},
	{168, "PINBALL.DAT"},
	{169, "Space Cadet"},
	{170, "Error:"},
	{171, "Unable to find other tables."},
	{172, "3D Pinball\nSpace Cadet"},
	{173, "Promotion to %s"},
	{174, "Demotion to %s"},
	{175, "Upgrade Attack Bumpers"},
	{176, "Fill Left Hazard Banks"},
	{177, "HIGH SCORE"},
	{178, "pinball.chm"},
	{179, "Not enough memory to run 3D Pinball."},
	{180, "Player 1's Score\n%ld"},
	{181, "Player 2's Score\n%ld"},
	{182, "Player 3's Score\n%ld"},
	{183, "Player 4's Score\n%ld"},
	{184, "High Score 1\n%ld"},
	{185, "High Score 2\n%ld"},
	{186, "High Score 3\n%ld"},
	{187, "High Score 4\n%ld"},
	{188, "High Score 5\n%ld"},
	{189, "255 255 255   (R G B default font color)"},
	{2030, "Use Maximum Resolution (640 x 480)"},
	{2031, "Use Maximum Resolution (800 x 600)"},
	{2032, "Use Maximum Resolution (1024 x 768)"}
};

int LoadStringAlt(uint32_t uID, LPSTR lpBuffer, int cchBufferMax)
{
	auto str = rc_strings.find(uID);
	if (str == rc_strings.end())
	{		
		return 0;
	}

	strncpy(lpBuffer, str->second, cchBufferMax);
	return 1;
}

int pinball::quickFlag = 0;
TTextBox* pinball::InfoTextBox;
TTextBox* pinball::MissTextBox;
char pinball::getRcBuffer[6 * 256];
int pinball::rc_string_slot = 0;
int pinball::LeftShift = -1;
int pinball::RightShift = -1;

char* pinball::get_rc_string(int uID, int a2)
{
	char* result = &getRcBuffer[256 * rc_string_slot];
	if (!LoadStringAlt(uID, &getRcBuffer[256 * rc_string_slot], 255))
		*result = 0;
	if (++rc_string_slot >= 6)
		rc_string_slot = 0;
	return result;
}

int pinball::get_rc_int(int uID, int* dst)
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
	return winmain::BasePath + fileName;
}
