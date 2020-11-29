#pragma once
#include "partman.h"
#include "TPinballTable.h"

class pb
{
public:
	static int time_ticks;
	static int ball_speed_limit;
	static datFileStruct* record_table;
	static TPinballTable* MainTable;

	static int init();
	static void reset_table();
	static void firsttime_setup();
	static void paint();
	static void mode_change(int mode);
	static void toggle_demo();
	static void replay_level(int demoMode);
	static void ballset(int x, int y);
	static int frame(int time);
private :
	static int demo_mode;
};
