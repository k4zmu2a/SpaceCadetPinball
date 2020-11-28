#pragma once
#include "TCollisionComponent.h"

class TDrain :
	public TCollisionComponent
{
public:
	TDrain(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
