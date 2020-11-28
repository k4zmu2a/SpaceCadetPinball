#pragma once
#include "TLightGroup.h"

class TLightBargraph :
	public TLightGroup
{
public:
	TLightBargraph(TPinballTable* table, int groupIndex) : TLightGroup(table, groupIndex)
	{
	}
};
