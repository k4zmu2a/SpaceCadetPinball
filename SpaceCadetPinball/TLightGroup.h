#pragma once
#include "TPinballComponent.h"
class TLightGroup :
    public TPinballComponent
{
public:
	TLightGroup(TPinballTable* table,int ind) : TPinballComponent(table, -1, false)
	{
	}
};

