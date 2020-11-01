#pragma once
#include "TPinballComponent.h"
class TGate :
    public TPinballComponent
{
public:
	TGate(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

