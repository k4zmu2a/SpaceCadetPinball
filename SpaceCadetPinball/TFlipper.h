#pragma once
#include "TCollisionComponent.h"

class TFlipper :
	public TCollisionComponent
{
public:
	TFlipper(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
	{
	}
};
