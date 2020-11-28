#pragma once
#include "TCollisionComponent.h"

class TTableLayer :
	public TCollisionComponent
{
public:
	TTableLayer(TPinballTable* table) : TCollisionComponent(table, -1, false)
	{
	}
};
