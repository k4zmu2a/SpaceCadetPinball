#pragma once

struct render_sprite_type_struct;
struct component_control;
class TPinballTable;
class TZmapList;

enum class message_code
{
	Reset = 1024,
};

class TPinballComponent
{
public:
	TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals);
	virtual ~TPinballComponent();
	virtual int Message(int code, float value);
	virtual void port_draw();
	virtual void put_scoring(int scoreId, int value);
	virtual int get_scoring(int score1);

	__int8 UnknownBaseFlag1;
	__int8 UnknownBaseFlag2;
	int MessageField;
	char* GroupName;
	int Unknown4;
	component_control* Control;
	int GroupIndex;
	render_sprite_type_struct* RenderSprite;
	TPinballTable* PinballTable;
	TZmapList* ListBitmap;
	TZmapList* ListZMap;
};
