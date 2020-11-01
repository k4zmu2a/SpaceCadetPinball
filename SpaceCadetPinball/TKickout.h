#pragma once
#include "TPinballComponent.h"
class TKickout :
    public TPinballComponent
{
public:
	TKickout(TPinballTable* table, int groupIndex,int vectorType) : TPinballComponent(table, groupIndex, false)
	{
	}
};

