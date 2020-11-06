#pragma once
#include "TPinballTable.h"

class pb
{
public:
	static void reset_table();
	static void firsttime_setup();
private:
	static TPinballTable* MainTable;
};

