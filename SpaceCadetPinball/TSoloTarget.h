#pragma once
#include "TPinballComponent.h"
class TSoloTarget :
    public TPinballComponent
{
public:
	TSoloTarget(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

