#pragma once
#include "TCollisionComponent.h"

class TRollover :
	public TCollisionComponent
{
public:
	TRollover(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
