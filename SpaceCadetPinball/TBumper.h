#pragma once
#include "TPinballComponent.h"
class TBumper :
    public TPinballComponent
{
public:
	TBumper(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

