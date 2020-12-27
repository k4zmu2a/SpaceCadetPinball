#pragma once
#include "high_score.h"
#include "partman.h"
#include "TPinballTable.h"

class pb
{
public:
	static int time_ticks;
	static float ball_speed_limit;
	static int cheat_mode, game_mode;
	static datFileStruct* record_table;
	static TPinballTable* MainTable;

	static int init();
	static int uninit();
	static void reset_table();
	static void firsttime_setup();
	static void paint();
	static void mode_change(int mode);
	static void toggle_demo();
	static void replay_level(int demoMode);
	static void ballset(int x, int y);
	static int frame(int time);
	static void window_size(int* width, int* height);
	static void pause_continue();
	static void loose_focus();
	static void keyup(int key);
	static void keydown(int key);
	static void ctrl_bdoor_controller(int key);
	static int mode_countdown(int time);
	static int cheat_bump_rank();
	static void launch_ball();
	static int end_game();
	static void high_scores();
	static void tilt_no_more();
	static bool chk_highscore();
private :
	static int demo_mode, mode_countdown_;
	static float time_now, time_next;
	static high_score_struct highscore_table[5];
	static int state;
};
