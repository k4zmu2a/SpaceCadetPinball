#pragma once
#include "TPinballComponent.h"
class TRamp :
    public TPinballComponent
{
public:
	TRamp(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

