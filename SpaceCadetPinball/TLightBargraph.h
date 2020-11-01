#pragma once
#include "TPinballComponent.h"
class TLightBargraph :
    public TPinballComponent
{
public:
	TLightBargraph(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

