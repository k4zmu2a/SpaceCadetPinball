#include "pch.h"
#include "TPlunger.h"


#include "control.h"
#include "loader.h"
#include "maths.h"
#include "pb.h"
#include "render.h"
#include "TBall.h"
#include "timer.h"
#include "TPinballTable.h"

TPlunger::TPlunger(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	Boost = 0.0;
	BallFeedTimer_ = 0;
	PullbackTimer_ = 0;
	SoundIndexP1 = visual.SoundIndex4;
	SoundIndexP2 = visual.SoundIndex3;
	HardHitSoundId = visual.Kicker.HardHitSoundId;
	Threshold = 1000000000.0;
	MaxPullback = 100;
	Elasticity = 0.5f;
	Smoothness = 0.5f;
	PullbackIncrement = static_cast<int>(100.0 / (ListBitmap->size() * 8.0));
	Unknown4F = 0.025f;
	float* floatArr = loader::query_float_attribute(groupIndex, 0, 601);
	table->PlungerPositionX = floatArr[0];
	table->PlungerPositionY = floatArr[1];
}

void TPlunger::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	if (PinballTable->TiltLockFlag)
		Message(1017, 0.0);
	coef = RandFloat() * Boost * 0.1f + Boost; // it is intended that the passed in coef is never used!
	maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, Threshold, coef);
}

int TPlunger::Message(int code, float value)
{
	switch (code)
	{
	case 1004:
		if (!PullbackTimer_)
		{
			Boost = 0.0;
			Threshold = 1000000000.0;
			loader::play_sound(HardHitSoundId);
			PullbackTimer(0, this);
		}
		return 0;
	case 1015:
		{
			auto ball = PinballTable->BallList.at(0);
			ball->Message(1024, 0.0);
			ball->Position.X = PinballTable->PlungerPositionX;
			ball->Position.Y = PinballTable->PlungerPositionY;
			ball->ActiveFlag = 1;
			PinballTable->BallInSink = 0;
			pb::tilt_no_more();
			control::handler(code, this);
			return 0;
		}
	case 1016:
		if (BallFeedTimer_)
			timer::kill(BallFeedTimer_);
		BallFeedTimer_ = timer::set(0.95999998f, this, BallFeedTimer);
		loader::play_sound(SoundIndexP1);
		control::handler(code, this);
		return 0;
	case 1017:
		Threshold = 0.0;
		Boost = static_cast<float>(MaxPullback);
		timer::set(0.2f, this, PlungerReleasedTimer);
		break;
	case 1005:
	case 1009:
	case 1010:
	case 1024:
		{
			if (code == 1024)
			{
				if (BallFeedTimer_)
					timer::kill(BallFeedTimer_);
				BallFeedTimer_ = 0;
			}

			Threshold = 0.0;
			if (PullbackTimer_)
				timer::kill(PullbackTimer_);
			PullbackTimer_ = 0;
			if (code == 1005)
				loader::play_sound(SoundIndexP2);
			auto bmp = ListBitmap->at(0);
			auto zMap = ListZMap->at(0);
			render::sprite_set(
				RenderSprite,
				bmp,
				zMap,
				bmp->XPosition - PinballTable->XOffset,
				bmp->YPosition - PinballTable->YOffset);

			timer::set(Unknown4F, this, PlungerReleasedTimer);
			break;
		}
	default:
		break;
	}
	return 0;
}

void TPlunger::BallFeedTimer(int timerId, void* caller)
{
	auto plunger = static_cast<TPlunger*>(caller);
	plunger->PullbackTimer_ = 0;
	plunger->Message(1015, 0.0);
}

void TPlunger::PullbackTimer(int timerId, void* caller)
{
	auto plunger = static_cast<TPlunger*>(caller);
	plunger->Boost += static_cast<float>(plunger->PullbackIncrement);
	if (plunger->Boost <= static_cast<float>(plunger->MaxPullback))
	{
		plunger->PullbackTimer_ = timer::set(plunger->Unknown4F, plunger, PullbackTimer);
	}
	else
	{
		plunger->PullbackTimer_ = 0;
		plunger->Boost = static_cast<float>(plunger->MaxPullback);
	}
	int index = static_cast<int>(floor(
		static_cast<float>(plunger->ListBitmap->size() - 1) *
		(plunger->Boost / static_cast<float>(plunger->MaxPullback))));
	auto bmp = plunger->ListBitmap->at(index);
	auto zMap = plunger->ListZMap->at(index);
	render::sprite_set(
		plunger->RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - plunger->PinballTable->XOffset,
		bmp->YPosition - plunger->PinballTable->YOffset);
}

void TPlunger::PlungerReleasedTimer(int timerId, void* caller)
{
	auto plunger = static_cast<TPlunger*>(caller);
	plunger->Threshold = 1000000000.0;
	plunger->Boost = 0.0;
}
