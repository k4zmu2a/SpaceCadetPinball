#include "pch.h"
#include "TFlipper.h"


#include "control.h"
#include "loader.h"
#include "pb.h"
#include "render.h"
#include "TBall.h"
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
	flipperEdge->place_in_grid(&AABB);

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

		MessageField = FlipperEdge->SetMotion(code);
		break;
	case MessageCode::PlayerChanged:
	case MessageCode::Reset:
		if (MessageField)
		{
			FlipperEdge->CurrentAngle = 0;
			FlipperEdge->set_control_points(0);
			MessageField = 0;
			FlipperEdge->SetMotion(MessageCode::Reset);
			UpdateSprite();
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

void TFlipper::UpdateSprite()
{
	auto bmpCountSub1 = static_cast<int>(ListBitmap->size()) - 1;
	auto newBmpIndex = static_cast<int>(floor(FlipperEdge->CurrentAngle / FlipperEdge->AngleMax * bmpCountSub1 + 0.5f));
	newBmpIndex = Clamp(newBmpIndex, 0, bmpCountSub1);
	if (BmpIndex == newBmpIndex)
		return;

	BmpIndex = newBmpIndex;
	SpriteSet(BmpIndex);
}

int TFlipper::GetFlipperStepAngle(float dt, float* dst) const
{
	if (!MessageField)
		return 0;

	auto deltaAngle = FlipperEdge->flipper_angle_delta(dt);
	auto step = std::fabs(std::ceil(FlipperEdge->DistanceDiv * deltaAngle * FlipperEdge->InvT1Radius));
	if (step > 3.0f)
		step = 3.0f;
	if (step >= 2.0f)
	{
		*dst = deltaAngle / step;
		return static_cast<int>(step);
	}

	*dst = deltaAngle;
	return 1;
}

void TFlipper::FlipperCollision(float deltaAngle)
{
	if (!MessageField)
		return;

	ray_type ray{}, rayDst{};
	ray.MinDistance = 0.002f;
	bool collisionFlag = false;
	for (auto ball : pb::MainTable->BallList)
	{
		if ((FlipperEdge->CollisionGroup & ball->CollisionMask) != 0 &&
			FlipperEdge->YMax >= ball->Position.Y && FlipperEdge->YMin <= ball->Position.Y &&
			FlipperEdge->XMax >= ball->Position.X && FlipperEdge->XMin <= ball->Position.X)
		{
			if (FlipperEdge->ControlPointDirtyFlag)
				FlipperEdge->set_control_points(FlipperEdge->CurrentAngle);
			ray.CollisionMask = ball->CollisionMask;
			ray.Origin = ball->Position;

			float sin, cos;
			auto ballPosRot = ray.Origin;
			maths::SinCos(-deltaAngle, sin, cos);
			maths::RotatePt(ballPosRot, sin, cos, FlipperEdge->RotOrigin);
			ray.Direction.X = ballPosRot.X - ray.Origin.X;
			ray.Direction.Y = ballPosRot.Y - ray.Origin.Y;
			ray.MaxDistance = maths::normalize_2d(ray.Direction);
			auto distance = maths::distance_to_flipper(FlipperEdge, ray, rayDst);
			if (distance < 1e9f)
			{
				FlipperEdge->NextBallPosition = ball->Position;
				FlipperEdge->CollisionDirection = rayDst.Direction;
				FlipperEdge->EdgeCollision(ball, distance);
				collisionFlag = true;
			}
		}
	}

	if (collisionFlag)
	{
		auto angleAdvance = deltaAngle / (std::fabs(FlipperEdge->MoveSpeed) * 5.0f);
		FlipperEdge->CurrentAngle -= angleAdvance;
		FlipperEdge->AngleRemainder += std::fabs(angleAdvance);
	}
	else
	{
		FlipperEdge->CurrentAngle += deltaAngle;
		FlipperEdge->AngleRemainder -= std::fabs(deltaAngle);
	}

	if (FlipperEdge->AngleRemainder <= 0.0001f)
	{
		FlipperEdge->CurrentAngle = FlipperEdge->AngleDst;
		FlipperEdge->FlipperFlag = MessageCode::TFlipperNull;
		MessageField = 0;
	}
	FlipperEdge->ControlPointDirtyFlag = true;
}
