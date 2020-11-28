#pragma once
#include "TCollisionComponent.h"

class THole :
	public TCollisionComponent
{
public:
	THole(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
	{
	}
};
