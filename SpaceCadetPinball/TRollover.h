#pragma once
#include "TPinballComponent.h"
class TRollover :
    public TPinballComponent
{
public:
	TRollover(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

