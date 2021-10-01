#include "pch.h"
#include "TPopupTarget.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TPopupTarget::TPopupTarget(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	this->Timer = 0;
	this->TimerTime = *loader::query_float_attribute(groupIndex, 0, 407);
}

int TPopupTarget::Message(int code, float value)
{
	switch (code)
	{
	case 49:
		this->ActiveFlag = 0;
		render::sprite_set_bitmap(this->RenderSprite, nullptr);
		break;
	case 50:
		this->Timer = timer::set(this->TimerTime, this, TimerExpired);
		break;
	case 1020:
		this->PlayerMessagefieldBackup[this->PinballTable->CurrentPlayer] = this->MessageField;
		this->MessageField = this->PlayerMessagefieldBackup[static_cast<int>(floor(value))];
		TPopupTarget::Message(50 - (MessageField != 0), 0.0);
		break;
	case 1024:
		{
			this->MessageField = 0;
			int* playerPtr = this->PlayerMessagefieldBackup;
			for (auto index = 0; index < this->PinballTable->PlayerCount; ++index)
			{
				*playerPtr = 0;
				++playerPtr;
			}

			if (this->Timer)
				timer::kill(this->Timer);
			TimerExpired(0, this);
			break;
		}
	default:
		break;
	}
	return 0;
}

void TPopupTarget::put_scoring(int index, int score)
{
	if (index < 3)
		Scores[index] = score;
}

int TPopupTarget::get_scoring(int index)
{
	return index < 3 ? Scores[index] : 0;
}

void TPopupTarget::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
                             TEdgeSegment* edge)
{
	if (this->PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, this->Elasticity, this->Smoothness, 1000000000.0, 0.0);
	}
	else if (maths::basic_collision(
		ball,
		nextPosition,
		direction,
		this->Elasticity,
		this->Smoothness,
		this->Threshold,
		this->Boost) > this->Threshold)
	{
		if (this->HardHitSoundId)
			loader::play_sound(this->HardHitSoundId);
		this->Message(49, 0.0);
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
			loader::play_sound(target->SoftHitSoundId);
	}
}
