#pragma once
#include "TPinballComponent.h"

class TTextBox :
	public TPinballComponent
{
public:
	TTextBox(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
	{
	}

	int Message(int a2, float a3) override;
};
