#pragma once
#include "TPinballComponent.h"
class TPopupTarget :
    public TPinballComponent
{
public:
	TPopupTarget(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
	{
	}
};

