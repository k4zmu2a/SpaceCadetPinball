#pragma once
#include "TPinballComponent.h"

class TPlunger :
	public TPinballComponent
{
public:
	TPlunger(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};
