#pragma once
#include "TCollisionComponent.h"

class TPlunger :
	public TCollisionComponent
{
public:
	TPlunger(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
