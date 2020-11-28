#pragma once
#include "TCollisionComponent.h"

class TBumper :
	public TCollisionComponent
{
public:
	TBumper(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
