#pragma once
#include "TPinballComponent.h"
class TSound :
    public TPinballComponent
{
public:
	TSound(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

