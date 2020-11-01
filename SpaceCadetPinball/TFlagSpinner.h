#pragma once
#include "TPinballComponent.h"
class TFlagSpinner :
    public TPinballComponent
{
public:
	TFlagSpinner(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

