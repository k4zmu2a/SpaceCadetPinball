#include "pch.h"
#include "TSoloTarget.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"
#include "TZmapList.h"

TSoloTarget::TSoloTarget(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	Timer = 0;
	TimerTime = 0.1f;
	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex4 = visual.SoundIndex4;
	TSoloTarget::Message(50, 0.0);
}

int TSoloTarget::Message(int code, float value)
{
	switch (code)
	{
	case 49:
	case 50:
		UnknownBaseFlag2 = code == 50;
		break;
	case 1024:
		if (Timer)
			timer::kill(Timer);
		Timer = 0;
		UnknownBaseFlag2 = 1;
		break;
	default:
		return 0;
	}

	if (ListBitmap)
	{
		auto index = 1 - UnknownBaseFlag2;
		auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(index));
		auto zMap = static_cast<zmap_header_type*>(ListZMap->Get(index));
		render::sprite_set(
			RenderSprite,
			bmp,
			zMap,
			bmp->XPosition - PinballTable->XOffset,
			bmp->YPosition - PinballTable->YOffset);
	}

	return 0;
}

void TSoloTarget::put_scoring(int index, int score)
{
	if (index < 1)
		Scores[index] = score;
}

int TSoloTarget::get_scoring(int index)
{
	return index < 1 ? Scores[index] : 0;
}

void TSoloTarget::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
                            TEdgeSegment* edge)
{
	if (DefaultCollision(ball, nextPosition, direction))
	{
		Message(49, 0.0);
		Timer = timer::set(TimerTime, this, TimerExpired);
		control::handler(63, this);
	}
}

void TSoloTarget::TimerExpired(int timerId, void* caller)
{
	auto target = static_cast<TSoloTarget*>(caller);
	target->Message(50, 0.0);
	target->Timer = 0;
}
