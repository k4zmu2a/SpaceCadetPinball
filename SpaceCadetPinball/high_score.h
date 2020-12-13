#pragma once
#include "pinball.h"

struct high_score_struct
{
	char Name[32];
	int Score;
};


class high_score
{
public:
	static int read(high_score_struct* table, int* ptrToSmth);
	static int write(high_score_struct* table, int* ptrToSmth);
	static void clear_table(high_score_struct* table);
	static int get_score_position(high_score_struct* table, int score);
	static int place_new_score_into(high_score_struct* table, int score, LPSTR scoreStr, int position);
	static void scramble_number_string(int Value, char* Buffer);

	static void show_high_score_dialog(high_score_struct* table);
	static void show_and_set_high_score_dialog(high_score_struct* table, int score, int pos, LPCSTR defaultName);
	static INT_PTR __stdcall HighScore(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void show_high_scores(HWND hDlg, high_score_struct* table);
	static void hsdlg_show_score(HWND hDlg, LPCSTR name, int score, int position);
private :
	static int dlg_enter_name;
	static int dlg_score;
	static int position;
	static LPCSTR default_name;
	static high_score_struct* dlg_hst;
	static winhelp_entry help[21];
};
