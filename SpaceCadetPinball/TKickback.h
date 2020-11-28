#pragma once
#include "TCollisionComponent.h"

class TKickback :
	public TCollisionComponent
{
public:
	TKickback(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
