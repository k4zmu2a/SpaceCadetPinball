#include "pch.h"
#include "TFlipper.h"


#include "control.h"
#include "loader.h"
#include "pb.h"
#include "render.h"
#include "TFlipperEdge.h"
#include "timer.h"
#include "TPinballTable.h"

TFlipper::TFlipper(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	HardHitSoundId = visual.SoundIndex4;
	SoftHitSoundId = visual.SoundIndex3;
	Elasticity = visual.Elasticity;
	Smoothness = visual.Smoothness;

	auto collMult = *loader::query_float_attribute(groupIndex, 0, 803);
	auto retractTime = *loader::query_float_attribute(groupIndex, 0, 805);
	auto extendTime = *loader::query_float_attribute(groupIndex, 0, 804);
	auto vecT2 = reinterpret_cast<vector3*>(loader::query_float_attribute(groupIndex, 0, 802));
	auto vecT1 = reinterpret_cast<vector3*>(loader::query_float_attribute(groupIndex, 0, 801));
	auto origin = reinterpret_cast<vector3*>(loader::query_float_attribute(groupIndex, 0, 800));
	auto flipperEdge = new TFlipperEdge(
		this,
		&ActiveFlag,
		visual.CollisionGroup,
		table,
		origin,
		vecT1,
		vecT2,
		extendTime,
		retractTime,
		collMult,
		Elasticity,
		Smoothness);

	FlipperEdge = flipperEdge;
	BmpIndex = 0;
	if (table)
		table->FlipperList.push_back(this);
}

TFlipper::~TFlipper()
{
	delete FlipperEdge;
	if (PinballTable)
	{
		auto& flippers = PinballTable->FlipperList;
		auto position = std::find(flippers.begin(), flippers.end(), this);
		if (position != flippers.end())
			flippers.erase(position);
	}
}

int TFlipper::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TFlipperExtend:
	case MessageCode::TFlipperRetract:
	case MessageCode::Resume:
	case MessageCode::LooseFocus:
	case MessageCode::SetTiltLock:
	case MessageCode::GameOver:
		if (code == MessageCode::TFlipperExtend)
		{
			control::handler(MessageCode::TFlipperExtend, this);
			loader::play_sound(HardHitSoundId, this, "TFlipper1");
		}
		else if (code == MessageCode::TFlipperRetract)
		{
			loader::play_sound(SoftHitSoundId, this, "TFlipper2");
		}
		else
		{
			// Retract for all non-input messages
			code = MessageCode::TFlipperRetract;
		}

		MessageField = FlipperEdge->SetMotion(~code, value);
		break;
	case MessageCode::PlayerChanged:
	case MessageCode::Reset:
		if (MessageField)
		{
			MessageField = 0;
			FlipperEdge->SetMotion(1024, value);
			UpdateSprite(0);
		}
		break;
	default: break;
	}

	return 0;
}

void TFlipper::port_draw()
{
	FlipperEdge->port_draw();
}

void TFlipper::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
}

void TFlipper::UpdateSprite(float timeNow)
{
	int bmpCountSub1 = ListBitmap->size() - 1;

	auto newBmpIndex = static_cast<int>(floor(FlipperEdge->flipper_angle(timeNow) / FlipperEdge->AngleMax * bmpCountSub1 + 0.5f));
	newBmpIndex = Clamp(newBmpIndex, 0, bmpCountSub1);
	if (BmpIndex == newBmpIndex)
		return;

	BmpIndex = newBmpIndex;
	auto bmp = ListBitmap->at(BmpIndex);
	auto zMap = ListZMap->at(BmpIndex);
	render::sprite_set(
		RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - PinballTable->XOffset,
		bmp->YPosition - PinballTable->YOffset);
}
