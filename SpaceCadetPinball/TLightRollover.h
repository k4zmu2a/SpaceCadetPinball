#pragma once
#include "TPinballComponent.h"
class TLightRollover :
    public TPinballComponent
{
public:
	TLightRollover(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

