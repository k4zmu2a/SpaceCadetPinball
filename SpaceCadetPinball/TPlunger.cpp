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
#include "TZmapList.h"

TPlunger::TPlunger(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	CollisionMultiplier = 0.0;
	BallFeedTimer_ = 0;
	PullbackTimer_ = 0;
	SoundIndexP1 = visual.SoundIndex4;
	SoundIndexP2 = visual.SoundIndex3;
	SoundIndex1 = visual.Kicker.SoundIndex;
	MaxCollisionSpeed = 1000000000.0;
	MaxPullback = 100;
	UnknownC4F = 0.5f;
	UnknownC5F = 0.5f;
	PullbackIncrement = static_cast<int>(100.0 / (ListBitmap->Count() * 8.0));
	Unknown4F = 0.025f;
	float* floatArr = loader::query_float_attribute(groupIndex, 0, 601);
	table->PlungerPositionX = floatArr[0];
	table->PlungerPositionY = floatArr[1];
}

void TPlunger::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	if (PinballTable->TiltLockFlag)
		Message(1017, 0.0);
	coef = static_cast<float>(rand()) * 0.00003051850947599719f * CollisionMultiplier * 0.1f + CollisionMultiplier;
	maths::basic_collision(ball, nextPosition, direction, UnknownC4F, UnknownC5F, MaxCollisionSpeed, coef);
}

int TPlunger::Message(int code, float value)
{
	switch (code)
	{
	case 1004:
		if (!PullbackTimer_)
		{
			CollisionMultiplier = 0.0;
			MaxCollisionSpeed = 1000000000.0;
			loader::play_sound(SoundIndex1);
			PullbackTimer(0, this);
		}
		return 0;
	case 1005:
	case 1009:
	case 1010:
		{
			MaxCollisionSpeed = 0.0;
			if (PullbackTimer_)
				timer::kill(PullbackTimer_);
			PullbackTimer_ = 0;
			if (code == 1005)
				loader::play_sound(SoundIndexP2);
			auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(0));
			auto zMap = static_cast<zmap_header_type*>(ListZMap->Get(0));
			render::sprite_set(
				RenderSprite,
				bmp,
				zMap,
				bmp->XPosition - PinballTable->XOffset,
				bmp->YPosition - PinballTable->YOffset);

			timer::set(Unknown4F, this, PlungerReleasedTimer);
			break;
		}
	case 1015:
		{
			auto ball = static_cast<TBall*>(PinballTable->ComponentList->Get(0));
			ball->Message(1024, 0.0);
			ball->Position.X = PinballTable->PlungerPositionX;
			ball->Position.Y = PinballTable->PlungerPositionY;
			ball->UnknownBaseFlag2 = 1;
			PinballTable->UnknownP10 = 0;
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
		MaxCollisionSpeed = 0.0;
		CollisionMultiplier = static_cast<float>(MaxPullback);
		timer::set(0.2f, this, PlungerReleasedTimer);
		break;
	case 1024:
		{
			if (BallFeedTimer_)
				timer::kill(BallFeedTimer_);
			BallFeedTimer_ = 0;
			MaxCollisionSpeed = 0.0;
			if (PullbackTimer_)
				timer::kill(PullbackTimer_);
			PullbackTimer_ = 0;
			if (code == 1005)
				loader::play_sound(SoundIndexP2);
			auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(0));
			auto zMap = static_cast<zmap_header_type*>(ListZMap->Get(0));
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
	plunger->CollisionMultiplier += static_cast<float>(plunger->PullbackIncrement);
	if (plunger->CollisionMultiplier <= static_cast<float>(plunger->MaxPullback))
	{
		plunger->PullbackTimer_ = timer::set(plunger->Unknown4F, plunger, PullbackTimer);
	}
	else
	{
		plunger->PullbackTimer_ = 0;
		plunger->CollisionMultiplier = static_cast<float>(plunger->MaxPullback);
	}
	int index = static_cast<int>(floor(
		static_cast<float>(plunger->ListBitmap->Count() - 1) *
		(plunger->CollisionMultiplier / static_cast<float>(plunger->MaxPullback))));
	auto bmp = static_cast<gdrv_bitmap8*>(plunger->ListBitmap->Get(index));
	auto zMap = static_cast<zmap_header_type*>(plunger->ListZMap->Get(index));
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
	plunger->MaxCollisionSpeed = 1000000000.0;
	plunger->CollisionMultiplier = 0.0;
}
