#pragma once
#include "TPinballComponent.h"

class TBall :
	public TPinballComponent
{
public :
	TBall(TPinballTable* table): TPinballComponent(table, -1, false)
	{
	}
};
