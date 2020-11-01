#include "pch.h"
#include "TPinballTable.h"


#include "loader.h"
#include "pinball.h"
#include "TBall.h"
#include "TBlocker.h"
#include "TBumper.h"
#include "TComponentGroup.h"
#include "TDemo.h"
#include "TFlagSpinner.h"
#include "TGate.h"
#include "THole.h"
#include "TKickback.h"
#include "TKickout.h"
#include "TLight.h"
#include "TLightBargraph.h"
#include "TLightGroup.h"
#include "TLightRollover.h"
#include "TOneway.h"
#include "TPopupTarget.h"
#include "TRamp.h"
#include "TRollover.h"
#include "TSink.h"
#include "TSoloTarget.h"
#include "TSound.h"
#include "TTableLayer.h"
#include "TTextBox.h"
#include "TTimer.h"
#include "TTripwire.h"

TPinballTable::TPinballTable(): TPinballComponent(nullptr, -1, false)
{
	int shortArrLength;

	//this->VfTable = (int)&TPinballTable::`vftable';
	ListP1 = new objlist_class(32, 16);
	ListP2 = new objlist_class(3, 1);
	this->Score1 = nullptr;
	this->ScoreBallcount = nullptr;
	this->ScorePlayerNumber1 = nullptr;
	this->UnknownP10 = 0;
	this->UnknownBaseFlag2 = 1;
	this->UnknownP83 = 0;
	this->UnknownP14 = 0;
	this->UnknownP13 = 0;
	this->UnknownP80 = 0;
	this->UnknownP15 = 0;
	this->UnknownP77 = 0;

	TBall* ballObj = new TBall(this);
	ListP2->Add(ballObj);
	if (ballObj)
		ballObj->UnknownBaseFlag2 = 0;
	TTableLayer* tableLayerObj = new TTableLayer(this);
	TLightGroup* lightGroupObj = new TLightGroup(this, 0);
	this->LightGroup = lightGroupObj;

	auto score1 = score::create("score1", pinball::render_background_bitmap);
	this->Score1 = score1;
	this->Score2 = score1;
	int scoreIndex = 1;
	auto scorePtr = &this->Score3_x3_at7;
	do
	{
		*scorePtr = score::dup(this->Score1, scoreIndex++);
		scorePtr += 7;
	}
	while (scoreIndex < 4);
	this->UnknownP45 = 0;
	this->UnknownP73 = 3;
	this->ScoreBallcount = (int*)score::create("ballcount1", pinball::render_background_bitmap);
	this->ScorePlayerNumber1 = (int*)score::create("player_number1", pinball::render_background_bitmap);
	int groupIndexObjects = loader::query_handle("table_objects");

	short* shortArr = loader::query_iattribute(groupIndexObjects, 1025, &shortArrLength);
	if (shortArrLength > 0)
	{
		for (int i = 0; i < shortArrLength / 2; ++i)
		{
			int objectType = *shortArr;
			short* shortArrPlus1 = shortArr + 1;
			int groupIndex = *shortArrPlus1;
			shortArr = shortArrPlus1 + 1;
			switch (objectType)
			{
			case 1000:
			case 1010:
				// auto wall = new TWall( this, groupIndex);
				break;
			case 1001:
				this->Plunger = new TPlunger(this, groupIndex);
				break;
			case 1002:
				new TLight(this, groupIndex);
				//((objlist_class*)(this->LightGroup + 42))->Add(light);
				break;
			case 1003:
				this->FlipperL = new TFlipper(this, groupIndex);
				break;
			case 1004:
				this->FlipperR = new TFlipper(this, groupIndex);
				break;
			case 1005:
				new TBumper(this, groupIndex);
				break;
			case 1006:
				new TPopupTarget(this, groupIndex);
				break;
			case 1007:
				this->Drain = new TDrain(this, groupIndex);
				break;
			case 1011:
				new TBlocker(this, groupIndex);
				break;
			case 1012:
				new TKickout(this, groupIndex, 1);
				break;
			case 1013:
				new TGate(this, groupIndex);
				break;
			case 1014:
				new TKickback(this, groupIndex);
				break;
			case 1015:
				new TRollover(this, groupIndex);
				break;
			case 1016:
				new TOneway(this, groupIndex);
				break;
			case 1017:
				new TSink(this, groupIndex);
				break;
			case 1018:
				new TFlagSpinner(this, groupIndex);
				break;
			case 1019:
				new TSoloTarget(this, groupIndex);
				break;
			case 1020:
				new TLightRollover(this, groupIndex);
				break;
			case 1021:
				new TRamp(this, groupIndex);
				break;
			case 1022:
				new THole(this, groupIndex);
				break;
			case 1023:
				new TDemo(this, groupIndex);
				break;
			case 1024:
				new TTripwire(this, groupIndex);
				break;
			case 1026:
				new TLightGroup(this, groupIndex);
				break;
			case 1028:
				new TComponentGroup(this, groupIndex);
				break;
			case 1029:
				new TKickout(this, groupIndex, 0);
				break;
			case 1030:
				new TLightBargraph(this, groupIndex);
				break;
			case 1031:
				new TSound(this, groupIndex);
				break;
			case 1032:
				new TTimer(this, groupIndex);
				break;
			case 1033:
				new TTextBox(this, groupIndex);
				break;
			default:
				continue;
			}
		}
	}

	/* build_occlude_list();
	 InfoTextBox = (TTextBox*)TPinballTable::find_component(this, "info_text_box");
	 MissTextBox = (TTextBox*)TPinballTable::find_component(this, "mission_text_box");
	 control_make_links(this);*/
}
