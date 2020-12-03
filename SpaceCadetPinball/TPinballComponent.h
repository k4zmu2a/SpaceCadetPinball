#pragma once
#include "render.h"
#include "TZmapList.h"

class TPinballTable;

class TPinballComponent
{
public:
	TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals);
	virtual ~TPinballComponent();
	virtual int Message(int code, float value);
	virtual void port_draw();
	virtual void put_scoring(int score1, int score2);
	virtual int get_scoring(int score1);
	static void control_handler(int code, TPinballComponent* cmp);

	int VfTable;
	__int8 UnknownBaseFlag1;
	__int8 UnknownBaseFlag2;
	int MessageField;
	char* GroupName;
	int Unknown4;
	int Unknown5;
	int GroupIndex;
	render_sprite_type_struct* RenderSprite;
	TPinballTable* PinballTable;
	TZmapList* ListBitmap;
	TZmapList* ListZMap;
};
