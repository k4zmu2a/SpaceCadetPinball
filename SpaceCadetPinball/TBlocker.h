#pragma once
#include "TPinballComponent.h"
class TBlocker :
    public TPinballComponent
{
public:
	TBlocker(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

