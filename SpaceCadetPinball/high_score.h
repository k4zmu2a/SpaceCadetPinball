#pragma once
class high_score
{
public:
	static int read(CHAR* table, int* ptrToSmth);
	static int write(CHAR* table, int* ptrToSmth);
	static void show_high_score_dialog(CHAR* table);
};
