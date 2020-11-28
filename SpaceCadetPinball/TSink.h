#pragma once
#include "TCollisionComponent.h"

class TSink :
	public TCollisionComponent
{
public:
	TSink(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
