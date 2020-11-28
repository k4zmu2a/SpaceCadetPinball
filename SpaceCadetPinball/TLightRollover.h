#pragma once
#include "TRollover.h"

class TLightRollover :
	public TRollover
{
public:
	TLightRollover(TPinballTable* table, int groupIndex) : TRollover(table, groupIndex)
	{
	}
};
