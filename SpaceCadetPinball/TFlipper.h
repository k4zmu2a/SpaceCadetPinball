#pragma once
#include "TPinballComponent.h"
class TFlipper :
    public TPinballComponent
{
public:
	TFlipper(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

