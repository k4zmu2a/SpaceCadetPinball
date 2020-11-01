#pragma once
#include "TPinballComponent.h"
class TTableLayer :
    public TPinballComponent
{
public:
	TTableLayer(TPinballTable* table) : TPinballComponent(table, -1, false)
	{
	}
};

