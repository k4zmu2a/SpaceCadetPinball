#pragma once
#include "TPinballComponent.h"
class TSink :
    public TPinballComponent
{
public:
	TSink(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

