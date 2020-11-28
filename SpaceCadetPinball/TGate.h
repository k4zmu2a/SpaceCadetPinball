#pragma once
#include "TCollisionComponent.h"

class TGate :
	public TCollisionComponent
{
public:
	TGate(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
