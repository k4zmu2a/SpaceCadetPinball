#pragma once
#include "TCollisionComponent.h"

class TRamp :
	public TCollisionComponent
{
public:
	TRamp(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
	{
	}
};
