#include "pch.h"
#include "TPinballTable.h"


#include "control.h"
#include "loader.h"
#include "memory.h"
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
#include "TWall.h"

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
	new TTableLayer(this);
	this->LightGroup = new TLightGroup(this, 0);

	auto score1 = score::create("score1", render::background_bitmap);
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
	this->CurrentPlayer = 0;
	this->UnknownP73 = 3;
	this->ScoreBallcount = (int*)score::create("ballcount1", render::background_bitmap);
	this->ScorePlayerNumber1 = (int*)score::create("player_number1", render::background_bitmap);
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
				new TWall(this, groupIndex);
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

	//build_occlude_list();
	pinball::InfoTextBox = dynamic_cast<TTextBox*>(find_component("info_text_box"));
	pinball::MissTextBox = dynamic_cast<TTextBox*>(find_component("mission_text_box"));
	control::make_links(this);
}


TPinballTable::~TPinballTable()
{
	scoreStruct** scorePtr = &Score2;
	int index = 4;
	do
	{
		if (*scorePtr)
		{
			memory::free(*scorePtr);
			*scorePtr = nullptr;
		}
		scorePtr += 7;
		--index;
	}
	while (index);
	if (ScorePlayerNumber1)
	{
		memory::free(ScorePlayerNumber1);
		ScorePlayerNumber1 = nullptr;
	}
	if (ScoreBallcount)
	{
		memory::free(ScoreBallcount);
		ScoreBallcount = nullptr;
	}
	delete LightGroup;
	while (ListP1->Count() > 0)
	{
		delete static_cast<TPinballComponent*>(ListP1->Get(0));
	}
	delete ListP2;
	delete ListP1;
}

TPinballComponent* TPinballTable::find_component(LPCSTR componentName)
{
	int objCount = ListP1->Count();
	if (objCount > 0)
	{
		for (int index = 0; index < objCount; ++index)
		{
			TPinballComponent* obj = static_cast<TPinballComponent*>(ListP1->Get(index));
			const CHAR* groupName = obj->GroupName;
			if (groupName && !lstrcmpA(groupName, componentName))
			{
				return obj;
			}
		}
	}
	MessageBoxA(nullptr, "Table cant find:", componentName, 0x2000u);
	return nullptr;
}

TPinballComponent* TPinballTable::find_component(int groupIndex)
{
	char Buffer[33];
	int objCount = ListP1->Count();
	if (objCount > 0)
	{
		for (int index = 0; index < objCount; ++index)
		{
			TPinballComponent* obj = static_cast<TPinballComponent*>(ListP1->Get(index));
			if (obj->GroupIndex == groupIndex)
				return obj;
		}
	}
	_itoa_s(groupIndex, Buffer, 10);
	MessageBoxA(nullptr, "Table cant find (lh):", Buffer, 0x2000u);
	return nullptr;
}


void TPinballTable::port_draw()
{
	for (int index = ListP1->Count() - 1; index >= 0; index--)
	{
		static_cast<TPinballComponent*>(ListP1->Get(index))->port_draw();
	}
}
