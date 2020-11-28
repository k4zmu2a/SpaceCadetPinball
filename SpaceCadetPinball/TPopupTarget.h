#pragma once
#include "TCollisionComponent.h"

class TPopupTarget :
	public TCollisionComponent
{
public:
	TPopupTarget(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
	{
	}
};
