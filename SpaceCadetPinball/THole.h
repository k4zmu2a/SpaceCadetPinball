#pragma once
#include "TPinballComponent.h"
class THole :
    public TPinballComponent
{
public:
	THole(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

