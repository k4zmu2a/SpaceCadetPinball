#pragma once
#include "high_score.h"
#include "partman.h"
#include "TPinballTable.h"

class TBall;

class pb
{
public:
	static int time_ticks;
	static float ball_speed_limit, time_now, time_next;
	static int cheat_mode, game_mode;
	static datFileStruct* record_table;
	static TPinballTable* MainTable;
	static high_score_struct highscore_table[5];
	static bool FullTiltMode;

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
	static void timed_frame(float timeNow, float timeDelta, bool drawBalls);
	static void window_size(int* width, int* height);
	static void pause_continue();
	static void loose_focus();
	static void keyup(int key);
	static void keydown(int key);
	static int mode_countdown(int time);
	static void launch_ball();
	static void end_game();
	static void high_scores();
	static void tilt_no_more();
	static bool chk_highscore();
	static float collide(float timeNow, float timeDelta, TBall* ball);
private:
	static int demo_mode, mode_countdown_;
};
