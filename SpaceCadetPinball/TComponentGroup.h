#pragma once
#include "TPinballComponent.h"
class TComponentGroup :
    public TPinballComponent
{
public:
	TComponentGroup(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

