#include "pch.h"
#include "TPopupTarget.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TPopupTarget::TPopupTarget(TPinballTable* table, int groupIndex) : TCollisionComponent2(table, groupIndex, true)
{
	Timer = 0;
	TimerTime = *loader::query_float_attribute(groupIndex, 0, 407);
}

int TPopupTarget::Message2(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TPopupTargetDisable:
		ActiveFlag = 0;
		render::sprite_set_bitmap(RenderSprite, nullptr);
		break;
	case MessageCode::TPopupTargetEnable:
		Timer = timer::set(TimerTime, this, TimerExpired);
		break;
	case MessageCode::PlayerChanged:
		PlayerMessagefieldBackup[PinballTable->CurrentPlayer] = MessageField;
		MessageField = PlayerMessagefieldBackup[static_cast<int>(floor(value))];
		TPopupTarget::Message2(MessageField ? MessageCode::TPopupTargetDisable : MessageCode::TPopupTargetEnable, 0.0);
		break;
	case MessageCode::Reset:
	{
			MessageField = 0;
			int* playerPtr = PlayerMessagefieldBackup;
			for (auto index = 0; index < PinballTable->PlayerCount; ++index)
			{
				*playerPtr = 0;
				++playerPtr;
			}

			if (Timer)
				timer::kill(Timer);
			TimerExpired(0, this);
			break;
		}
	default:
		break;
	}
	return 0;
}

void TPopupTarget::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
                             TEdgeSegment* edge)
{
	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 1000000000.0, 0.0);
	}
	else if (maths::basic_collision(
		ball,
		nextPosition,
		direction,
		Elasticity,
		Smoothness,
		Threshold,
		Boost) > Threshold)
	{
		if (HardHitSoundId)
			loader::play_sound(HardHitSoundId, this, "TPopupTarget1");
		Message2(MessageCode::TPopupTargetDisable, 0.0);
		control::handler(63, this);
	}
}

void TPopupTarget::TimerExpired(int timerId, void* caller)
{
	auto target = static_cast<TPopupTarget*>(caller);
	target->Timer = 0;
	target->ActiveFlag = 1;
	render::sprite_set_bitmap(target->RenderSprite, target->ListBitmap->at(0));
	if (timerId)
	{
		if (target->SoftHitSoundId)
			loader::play_sound(target->SoftHitSoundId, target, "TPopupTarget2");
	}
}
