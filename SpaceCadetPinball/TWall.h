#pragma once
#include "TCollisionComponent.h"

class TWall :
	public TCollisionComponent
{
public:
	TWall(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
