#pragma once
#include "TPinballComponent.h"
class TDemo :
    public TPinballComponent
{
public:
	TDemo(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

