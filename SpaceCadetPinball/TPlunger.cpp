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

	// In FT, default max pullback is 50.
	if (pb::FullTiltMode)
		MaxPullback = 50;
	else
		MaxPullback = 100;

	Elasticity = 0.5f;
	Smoothness = 0.5f;
	PullbackIncrement = MaxPullback / (ListBitmap->size() * 8.0f);
	PullbackDelay = 0.025f;
	float* floatArr = loader::query_float_attribute(groupIndex, 0, 601);
	table->PlungerPositionX = floatArr[0];
	table->PlungerPositionY = floatArr[1];
}

void TPlunger::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	if (PinballTable->TiltLockFlag || SomeCounter > 0) 
	{
		auto boost = RandFloat() * MaxPullback * 0.1f + MaxPullback;
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 0, boost);
		if (SomeCounter)
			SomeCounter--;
		Message(MessageCode::PlungerInputReleased, 0.0);
	}
	else 
	{
		auto boost = RandFloat() * Boost * 0.1f + Boost;
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, Threshold, boost);
	}
}

int TPlunger::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::PlungerInputPressed:
		if (!PullbackStartedFlag && PinballTable->MultiballCount > 0 && !PinballTable->TiltLockFlag)
		{
			PullbackStartedFlag = true;
			Boost = 0.0;
			Threshold = 1000000000.0;
			loader::play_sound(HardHitSoundId, this, "TPlunger1");
			PullbackTimer(0, this);
		}
		break;
	case MessageCode::PlungerFeedBall:
		{
			RectF rect{};
			rect.XMin = PinballTable->CollisionCompOffset * -1.2f + PinballTable->PlungerPositionX;
			rect.XMax = PinballTable->CollisionCompOffset * 1.2f + PinballTable->PlungerPositionX;
			rect.YMin = PinballTable->CollisionCompOffset * -1.2f + PinballTable->PlungerPositionY;
			rect.YMax = PinballTable->CollisionCompOffset * 1.2f + PinballTable->PlungerPositionY;
			if(PinballTable->BallCountInRect(rect))
			{
				timer::set(1.0f, this, BallFeedTimer);
			}
			else 
			{
				auto ball = PinballTable->AddBall(PinballTable->PlungerPositionX, PinballTable->PlungerPositionY);
				assertm(ball, "Failure to create ball in plunger");
				PinballTable->MultiballCount++;
				PinballTable->BallInDrainFlag = 0;
				pb::tilt_no_more();
			}
			break;
		}
	case MessageCode::PlungerStartFeedTimer:
		timer::set(0.95999998f, this, BallFeedTimer);
		loader::play_sound(SoundIndexP1, this, "TPlunger2");
		break;
	case MessageCode::PlungerLaunchBall:
		PullbackStartedFlag = true;
		Boost = MaxPullback;
		Message(MessageCode::PlungerInputReleased, 0.0f);
		break;
	case MessageCode::PlungerRelaunchBall:
		SomeCounter++;
		timer::set(value, this, BallFeedTimer);
		loader::play_sound(SoundIndexP1, this, "TPlunger2_1");
		PullbackStartedFlag = true;
		PullbackTimer(0, this);
		break;
	case MessageCode::PlayerChanged:
		PullbackStartedFlag = false;
		Boost = 0.0f;
		Threshold = 1000000000.0f;
		SomeCounter = 0;
		timer::kill(BallFeedTimer);
		timer::kill(PullbackTimer);
		timer::kill(ReleasedTimer);
		break;
	case MessageCode::SetTiltLock:
		SomeCounter = 0;
		timer::kill(BallFeedTimer);
		break;
	case MessageCode::PlungerInputReleased:
	case MessageCode::Resume:
	case MessageCode::LooseFocus:
		if (PullbackStartedFlag && !SomeCounter)
		{
			PullbackStartedFlag = false;
			Threshold = 0.0;
			if (PullbackTimer_)
				timer::kill(PullbackTimer_);
			PullbackTimer_ = 0;
			loader::play_sound(SoundIndexP2, this, "TPlunger3");
			auto bmp = ListBitmap->at(0);
			auto zMap = ListZMap->at(0);
			RenderSprite->set(
				bmp,
				zMap,
				bmp->XPosition - PinballTable->XOffset,
				bmp->YPosition - PinballTable->YOffset);

			timer::set(PullbackDelay, this, ReleasedTimer);
		}
		break;
	case MessageCode::Reset:
	{
		PullbackStartedFlag = false;
		Boost = 0.0f;
		Threshold = 1000000000.0f;
		SomeCounter = 0;

		timer::kill(BallFeedTimer);
		timer::kill(PullbackTimer);
		timer::kill(ReleasedTimer);
		
		auto bmp = ListBitmap->at(0);
		auto zMap = ListZMap->at(0);
		RenderSprite->set(
			bmp,
			zMap,
			bmp->XPosition - PinballTable->XOffset,
			bmp->YPosition - PinballTable->YOffset);
		break;
	}
	default:
		break;
	}

	control::handler(code, this);
	return 0;
}

void TPlunger::BallFeedTimer(int timerId, void* caller)
{
	auto plunger = static_cast<TPlunger*>(caller);
	plunger->Message(MessageCode::PlungerFeedBall, 0.0);
}

void TPlunger::PullbackTimer(int timerId, void* caller)
{
	auto plunger = static_cast<TPlunger*>(caller);
	plunger->Boost += plunger->PullbackIncrement;
	if (plunger->Boost <= plunger->MaxPullback)
	{
		if (plunger->SomeCounter)
		{
			plunger->PullbackTimer_ = timer::set(plunger->PullbackDelay / 4.0f, plunger, PullbackTimer);
		}
		else
		{
			plunger->PullbackTimer_ = timer::set(plunger->PullbackDelay, plunger, PullbackTimer);
		}
	}
	else
	{
		plunger->PullbackTimer_ = 0;
		plunger->Boost = plunger->MaxPullback;
	}

	int index = static_cast<int>(floor(
		static_cast<float>(plunger->ListBitmap->size() - 1) *
		(plunger->Boost / plunger->MaxPullback)));
	auto bmp = plunger->ListBitmap->at(index);
	auto zMap = plunger->ListZMap->at(index);
	plunger->RenderSprite->set(
		bmp,
		zMap,
		bmp->XPosition - plunger->PinballTable->XOffset,
		bmp->YPosition - plunger->PinballTable->YOffset);
}

void TPlunger::ReleasedTimer(int timerId, void* caller)
{
	auto plunger = static_cast<TPlunger*>(caller);
	plunger->Threshold = 1000000000.0;
	plunger->Boost = 0.0;
}
