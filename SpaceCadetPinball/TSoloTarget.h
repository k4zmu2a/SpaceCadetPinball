#pragma once
#include "TCollisionComponent.h"

class TSoloTarget :
	public TCollisionComponent
{
public:
	TSoloTarget(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
