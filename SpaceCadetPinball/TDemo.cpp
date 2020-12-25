#include "pch.h"
#include "TDemo.h"

#include "TPinballTable.h"

TDemo::TDemo(TPinballTable* table, int groupIndex)
	: TCollisionComponent(table, groupIndex, false)
{
	table->Demo = this;
}
