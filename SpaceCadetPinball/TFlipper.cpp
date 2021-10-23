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
	Timer = 0;
	Smoothness = visual.Smoothness;

	auto collMult = *loader::query_float_attribute(groupIndex, 0, 803);
	auto retractTime = *loader::query_float_attribute(groupIndex, 0, 805);
	auto extendTime = *loader::query_float_attribute(groupIndex, 0, 804);

	/*Full tilt hack: different flipper speed*/
	if (pb::FullTiltMode)
	{
		retractTime = 0.08f;
		extendTime = 0.04f;
	}
	auto vecT2 = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, 0, 802));
	auto vecT1 = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, 0, 801));
	auto origin = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, 0, 800));
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
	if (flipperEdge)
	{
		ExtendAnimationFrameTime = flipperEdge->ExtendTime / static_cast<float>(ListBitmap->size() - 1);
		RetractAnimationFrameTime = flipperEdge->RetractTime / static_cast<float>(ListBitmap->size() - 1);
	}
	BmpIndex = 0;
	InputTime = 0.0;
}

TFlipper::~TFlipper()
{
	delete FlipperEdge;
}

int TFlipper::Message(int code, float value)
{
	if (code == 1 || code == 2 || (code > 1008 && code <= 1011) || code == 1022)
	{
		float timerTime;
		int command = code;
		if (code == 1)
		{
			control::handler(1, this);
			TimerTime = ExtendAnimationFrameTime;
			loader::play_sound(HardHitSoundId);
		}
		else if (code == 2)
		{
			TimerTime = RetractAnimationFrameTime;
			loader::play_sound(SoftHitSoundId);
		}
		else
		{
			// Retract for all non-input messages
			command = 2;
			TimerTime = RetractAnimationFrameTime;
		}

		if (MessageField)
		{
			// Message arrived before animation is finished
			auto inputDt = value - FlipperEdge->InputTime;
			timerTime = inputDt - floor(inputDt / TimerTime) * TimerTime;
			if (timerTime < 0.0f)
				timerTime = 0.0;
		}
		else
		{
			timerTime = TimerTime;
		}

		MessageField = command;
		InputTime = value;
		if (Timer)
			timer::kill(Timer);
		Timer = timer::set(timerTime, this, TimerExpired);
		FlipperEdge->SetMotion(command, value);
	}

	if (code == 1020 || code == 1024)
	{
		if (MessageField)
		{
			if (Timer)
				timer::kill(Timer);
			BmpIndex = -1;
			MessageField = 2;
			TimerExpired(Timer, this);
			FlipperEdge->SetMotion(code, value);
		}
	}
	return 0;
}

void TFlipper::port_draw()
{
	FlipperEdge->port_draw();
}

void TFlipper::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
}

void TFlipper::TimerExpired(int timerId, void* caller)
{
	auto flip = static_cast<TFlipper*>(caller);
	int bmpCountSub1 = flip->ListBitmap->size() - 1;

	auto newBmpIndex = static_cast<int>(floor((pb::time_now - flip->InputTime) / flip->TimerTime));
	if (newBmpIndex > bmpCountSub1)
		newBmpIndex = bmpCountSub1;
	if (newBmpIndex < 0)
		newBmpIndex = 0;

	bool bmpIndexOutOfBounds = false;
	if (flip->MessageField == 1)
	{
		flip->BmpIndex = newBmpIndex;
		if (flip->BmpIndex >= bmpCountSub1)
		{
			flip->BmpIndex = bmpCountSub1;
			bmpIndexOutOfBounds = true;
		}
	}
	if (flip->MessageField == 2)
	{
		flip->BmpIndex = bmpCountSub1 - newBmpIndex;
		if (flip->BmpIndex <= 0)
		{
			flip->BmpIndex = 0;
			bmpIndexOutOfBounds = true;
		}
	}

	if (bmpIndexOutOfBounds)
	{
		flip->MessageField = 0;
		flip->Timer = 0;
	}
	else
	{
		flip->Timer = timer::set(flip->TimerTime, flip, TimerExpired);
	}

	auto bmp = flip->ListBitmap->at(flip->BmpIndex);
	auto zMap = flip->ListZMap->at(flip->BmpIndex);
	render::sprite_set(
		flip->RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - flip->PinballTable->XOffset,
		bmp->YPosition - flip->PinballTable->YOffset);
}
