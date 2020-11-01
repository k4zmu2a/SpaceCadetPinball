#pragma once
#include "TPinballComponent.h"
class TDrain :
    public TPinballComponent
{
public:
	TDrain(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

