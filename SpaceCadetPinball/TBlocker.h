#pragma once
#include "TCollisionComponent.h"

class TBlocker :
	public TCollisionComponent
{
public:
	TBlocker(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
