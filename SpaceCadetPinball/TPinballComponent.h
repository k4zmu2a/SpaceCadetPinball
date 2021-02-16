#pragma once
#include "objlist_class.h"

struct zmap_header_type;
struct gdrv_bitmap8;
struct render_sprite_type_struct;
struct component_control;
class TPinballTable;

enum class message_code
{
	Reset = 1024,
	LightActiveCount = 37,
	LightTotalCount = 38,
	LightSetMessageField = 23,
};

class TPinballComponent
{
public:
	TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals);
	virtual ~TPinballComponent();
	virtual int Message(int code, float value);
	virtual void port_draw();
	virtual void put_scoring(int index, int score);
	virtual int get_scoring(int index);

	void* operator new(size_t Size);
	void operator delete(void* p);

	char UnusedBaseFlag;
	char ActiveFlag;
	int MessageField;
	char* GroupName;
	int Unknown4;
	component_control* Control;
	int GroupIndex;
	render_sprite_type_struct* RenderSprite;
	TPinballTable* PinballTable;
	objlist_class<gdrv_bitmap8>* ListBitmap;
	objlist_class<zmap_header_type>* ListZMap;
};
