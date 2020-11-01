#pragma once
#include "TPinballComponent.h"
class TOneway :
    public TPinballComponent
{
public:
	TOneway(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

