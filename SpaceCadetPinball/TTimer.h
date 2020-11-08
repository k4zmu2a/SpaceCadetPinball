#pragma once
#include "TPinballComponent.h"
class TTimer :
    public TPinballComponent
{
public:
	TTimer(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
	{
	}
};

