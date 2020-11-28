#pragma once
#include "TRollover.h"

class TTripwire :
	public TRollover
{
public:
	TTripwire(TPinballTable* table, int groupIndex) : TRollover(table, groupIndex)
	{
	}
};
