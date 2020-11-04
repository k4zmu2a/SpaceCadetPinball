#pragma once
#include "TZmapList.h"


class TPinballComponent
{
public:
	TPinballComponent(class TPinballTable* table, int groupIndex, bool loadVisuals);
	virtual ~TPinballComponent();
	virtual int Message(int message1, float message2);
	virtual void put_scoring(int score1, int score2);
	virtual int get_scoring(int score1);

	int VfTable;
	__int8 UnknownBaseFlag1;
	__int8 UnknownBaseFlag2;
	int MessageField;
	char* GroupName;
	int Unknown4;
	int Unknown5;
	int GroupIndex;
	int Unknown7;
	TPinballTable* PinballTable;
	TZmapList* List1Bitmap8;
	TZmapList* List2Bitmap16;
};
