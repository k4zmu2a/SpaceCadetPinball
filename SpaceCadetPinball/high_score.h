#pragma once

struct high_score_struct
{
	char Name[32];
	int Score;
};


class high_score
{
public:
	static int read(high_score_struct* table);
	static int write(high_score_struct* table);
	static void clear_table(high_score_struct* table);
	static int get_score_position(high_score_struct* table, int score);
	static int place_new_score_into(high_score_struct* table, int score, LPSTR scoreStr, int position);

	static void show_high_score_dialog(high_score_struct* table);
	static void show_and_set_high_score_dialog(high_score_struct* table, int score, int pos, LPCSTR defaultName);
	static void RenderHighScoreDialog();
private :
	static int dlg_enter_name;
	static int dlg_score;
	static int dlg_position;
	static char default_name[32];
	static high_score_struct* dlg_hst;
	static bool ShowDialog;
};
