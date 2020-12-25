#pragma once
#include "TCollisionComponent.h"

class TDemo :
	public TCollisionComponent
{
public:
	TDemo(TPinballTable* table, int groupIndex);
};
