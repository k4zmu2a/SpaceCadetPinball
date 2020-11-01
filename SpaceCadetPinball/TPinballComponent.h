#pragma once
#include "TZmapList.h"


class TPinballComponent
{public:
	TPinballComponent(class TPinballTable* table, int groupIndex, bool loadVisuals);

	int VfTable;
	__int8 UnknownBaseFlag1;
	__int8 UnknownBaseFlag2;
	int Unknown2;
	char* GroupName;
	int Unknown4;
	int Unknown5;
	int GroupIndex;
	int Unknown7;
	TPinballTable* PinballTable;
	TZmapList* List1Bitmap8;
	TZmapList* List2Bitmap16;
};

