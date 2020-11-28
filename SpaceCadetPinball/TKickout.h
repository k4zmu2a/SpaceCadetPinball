#pragma once
#include "TCollisionComponent.h"

class TKickout :
	public TCollisionComponent
{
public:
	TKickout(TPinballTable* table, int groupIndex, int vectorType) : TCollisionComponent(table, groupIndex, false)
	{
	}
};
