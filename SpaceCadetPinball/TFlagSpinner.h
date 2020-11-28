#pragma once
#include "TCollisionComponent.h"

class TFlagSpinner :
	public TCollisionComponent
{
public:
	TFlagSpinner(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
	{
	}
};
