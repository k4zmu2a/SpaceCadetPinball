#pragma once
#include "TPinballComponent.h"
class TKickback :
    public TPinballComponent
{
public:
	TKickback(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

