#pragma once
#include "TCollisionComponent.h"

class TOneway :
	public TCollisionComponent
{
public:
	TOneway(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
	{
	}
};
