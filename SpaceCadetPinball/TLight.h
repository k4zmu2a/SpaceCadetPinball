#pragma once
#include "TPinballComponent.h"
class TLight :
    public TPinballComponent
{
public:
	TLight(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
	{
	}
};

