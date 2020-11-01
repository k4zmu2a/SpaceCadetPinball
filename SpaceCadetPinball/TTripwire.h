#pragma once
#include "TPinballComponent.h"
class TTripwire :
    public TPinballComponent
{
public:
	TTripwire(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

