#pragma once
#include "objlist_class.h"
#include "score.h"
#include "TDrain.h"
#include "TFlipper.h"
#include "TLightGroup.h"
#include "TPinballComponent.h"
#include "TPlunger.h"

class TPinballTable : public TPinballComponent
{
public:
	TPinballTable();
	~TPinballTable();
	TPinballComponent* find_component(LPCSTR componentName);
	TPinballComponent* find_component(int groupIndex);
	void port_draw() override;

	TFlipper* FlipperL;
	TFlipper* FlipperR;
	scoreStruct* Score1;
	int* ScoreBallcount;
	int* ScorePlayerNumber1;
	int UnknownP6;
	int UnknownP7;
	int UnknownP8;
	int UnknownP9;
	int UnknownP10;
	int UnknownP11;
	int UnknownP12;
	int UnknownP13;
	int UnknownP14;
	int UnknownP15;
	scoreStruct* Score2;
	int UnknownP17;
	int UnknownP18;
	int UnknownP19;
	int UnknownP20;
	int UnknownP21;
	int UnknownP22;
	scoreStruct* Score3_x3_at7;
	int UnknownP24;
	int UnknownP25;
	int UnknownP26;
	int UnknownP27;
	int UnknownP28;
	int UnknownP29;
	int UnknownP30;
	int UnknownP31;
	int UnknownP32;
	int UnknownP33;
	int UnknownP34;
	int UnknownP35;
	int UnknownP36;
	int UnknownP37;
	int UnknownP38;
	int UnknownP39;
	int UnknownP40;
	int UnknownP41;
	int UnknownP42;
	int UnknownP43;
	int UnknownP44;
	int UnknownP45;
	TPlunger* Plunger;
	TDrain* Drain;
	int UnknownP48;
	int XOffset;
	int YOffset;
	int UnknownP51;
	int UnknownP52;
	objlist_class* ListP1;
	objlist_class* ListP2;
	TLightGroup* LightGroup;
	int UnknownP58;
	int UnknownP59;
	int UnknownP60;
	float CollisionCompOffset;
	int UnknownP62;
	int UnknownP63;
	int UnknownP64;
	int UnknownP65;
	int UnknownP66;
	int UnknownP67;
	int UnknownP68;
	int UnknownP69;
	int UnknownP70;
	int UnknownP71;
	int UnknownP72;
	int UnknownP73;
	int UnknownP74;
	int UnknownP75;
	int UnknownP76;
	int UnknownP77;
	int UnknownP78;
	int UnknownP79;
	int UnknownP80;
	int UnknownP81;
	int UnknownP82;
	int UnknownP83;
};
