#include "pch.h"
#include "TFlipperEdge.h"


#include "pb.h"
#include "TLine.h"
#include "TPinballTable.h"
#include "TTableLayer.h"


TFlipperEdge::TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup,
                           TPinballTable* table,
                           vector3* origin, vector3* vecT1, vector3* vecT2, float extendSpeed, float retractSpeed,
                           float collMult, float elasticity, float smoothness): TEdgeSegment(
	collComp, activeFlag, collisionGroup)
{
	vector3 crossProd{}, vecOriginT1{}, vecOriginT2{};

	Elasticity = elasticity;
	Smoothness = smoothness;
	CollisionMult = collMult;

	T1Src = static_cast<vector2>(*vecT1);
	T2Src = static_cast<vector2>(*vecT2);
	RotOrigin.X = origin->X;
	RotOrigin.Y = origin->Y;

	CirclebaseRadius = origin->Z + table->CollisionCompOffset;
	CirclebaseRadiusMSq = CirclebaseRadius * 1.01f * (CirclebaseRadius * 1.01f);
	CirclebaseRadiusSq = CirclebaseRadius * CirclebaseRadius;

	CircleT1Radius = vecT1->Z + table->CollisionCompOffset;
	CircleT1RadiusMSq = CircleT1Radius * 1.01f * (CircleT1Radius * 1.01f);
	CircleT1RadiusSq = CircleT1Radius * CircleT1Radius;

	vecOriginT1.X = vecT1->X - origin->X;
	vecOriginT1.Y = vecT1->Y - origin->Y;
	vecOriginT1.Z = 0.0;
	maths::normalize_2d(vecOriginT1);

	vecOriginT2.X = vecT2->X - origin->X;
	vecOriginT2.Y = vecT2->Y - origin->Y;
	vecOriginT2.Z = 0.0;
	maths::normalize_2d(vecOriginT2);

	AngleMax = acos(maths::DotProduct(vecOriginT1, vecOriginT2));
	maths::cross(vecOriginT1, vecOriginT2, crossProd);
	if (crossProd.Z < 0.0f)
		AngleMax = -AngleMax;

	FlipperFlag = MessageCode::TFlipperNull;
	AngleDst = 0.0;

	// 3DPB and FT have different formats for flipper speed:
	// 3DPB: Time it takes for flipper to go from source to destination, in sec.
	// FT: Flipper movement speed, in radians per sec.
	if (!pb::FullTiltMode)
	{
		auto angleMax = std::abs(AngleMax);
		retractSpeed = angleMax / retractSpeed;
		extendSpeed = angleMax / extendSpeed;
	}
	ExtendSpeed = extendSpeed;
	RetractSpeed = retractSpeed;

	const vector2 perpOriginT1Cc = {-vecOriginT1.Y, vecOriginT1.X};
	A2Src.X = perpOriginT1Cc.X * CirclebaseRadius + origin->X;
	A2Src.Y = perpOriginT1Cc.Y * CirclebaseRadius + origin->Y;
	A1Src.X = perpOriginT1Cc.X * CircleT1Radius + vecT1->X;
	A1Src.Y = perpOriginT1Cc.Y * CircleT1Radius + vecT1->Y;

	const vector2 perpOriginT1C = {vecOriginT1.Y, -vecOriginT1.X};
	B1Src.X = perpOriginT1C.X * CirclebaseRadius + origin->X;
	B1Src.Y = perpOriginT1C.Y * CirclebaseRadius + origin->Y;
	B2Src.X = perpOriginT1C.X * CircleT1Radius + vecT1->X;
	B2Src.Y = perpOriginT1C.Y * CircleT1Radius + vecT1->Y;

	if (AngleMax < 0.0f)
	{
		std::swap(A1Src, B1Src);
		std::swap(A2Src, B2Src);
	}

	auto dx = vecT1->X - RotOrigin.X;
	auto dy = vecT1->Y - RotOrigin.Y;
	auto distance1 = sqrt(dy * dy + dx * dx) + table->CollisionCompOffset + vecT1->Z;
	DistanceDiv = distance1;
	DistanceDivSq = distance1 * distance1;
	InvT1Radius = 1.0f / CircleT1Radius * 1.5f;

	if (AngleMax <= 0.0f)
	{
		ExtendSpeed = -ExtendSpeed;
	}
	else
	{
		RetractSpeed = -RetractSpeed;
	}
	set_control_points(CurrentAngle);
}

void TFlipperEdge::port_draw()
{
	set_control_points(CurrentAngle);
}

float TFlipperEdge::FindCollisionDistance(ray_type* ray)
{
	ray_type dstRay{};
	if (ControlPointDirtyFlag)
		set_control_points(CurrentAngle);
	auto distance = maths::distance_to_flipper(this, *ray, dstRay);
	if (distance >= 1e9f)
		return 1e9f;

	NextBallPosition = dstRay.Origin;
	CollisionDirection = dstRay.Direction;
	return distance;
}

void TFlipperEdge::EdgeCollision(TBall* ball, float distance)
{
	if (FlipperFlag == MessageCode::TFlipperNull)
	{
		maths::basic_collision(
			ball,
			&NextBallPosition,
			&CollisionDirection,
			Elasticity,
			Smoothness,
			1e9f,
			0);
		return;
	}

	auto someProduct = (NextBallPosition.Y - T1.Y) * (RotOrigin.X - T1.X) -
		(NextBallPosition.X - T1.X) * (RotOrigin.Y - T1.Y);

	bool someFlag = false;
	if (someProduct <= 0)
	{
		if (AngleMax > 0)
			someFlag = true;
	}
	else if (AngleMax <= 0)
	{
		someFlag = true;
	}

	if (FlipperFlag == MessageCode::TFlipperRetract)
	{
		someFlag ^= true;
		CollisionLinePerp = LineB.PerpendicularC;
	}
	else
	{
		CollisionLinePerp = LineA.PerpendicularC;
	}

	auto dx = NextBallPosition.X - RotOrigin.X;
	auto dy = NextBallPosition.Y - RotOrigin.Y;
	auto distanceSq = dy * dy + dx * dx;
	if (someFlag)
	{
		float boost = 0;
		if (circlebase.RadiusSq * 1.01f < distanceSq)
		{
			auto v21 = std::fabs(MoveSpeed) * std::sqrt(distanceSq / DistanceDivSq);
			auto dot1 = maths::DotProduct(CollisionLinePerp, CollisionDirection);
			if (dot1 >= 0)
				boost = CollisionMult * dot1 * v21;
		}

		auto threshold = boost <= 0.0f ? 1e9f : -1.0f;
		maths::basic_collision(
			ball,
			&NextBallPosition,
			&CollisionDirection,
			Elasticity,
			Smoothness,
			threshold,
			boost);
	}
	else
	{
		auto elasticity = Elasticity;
		if (circlebase.RadiusSq * 1.01f < distanceSq)
			elasticity = (1.0f - std::sqrt(distanceSq / DistanceDivSq)) * Elasticity;
		maths::basic_collision(ball, &NextBallPosition, &CollisionDirection, elasticity, Smoothness, 1e9f, 0.0);
	}
}

void TFlipperEdge::place_in_grid(RectF* aabb)
{
	auto xMax = std::max(std::max(T2Src.X + CircleT1Radius, T1Src.X + CircleT1Radius), RotOrigin.X + CirclebaseRadius);
	auto yMax = std::max(std::max(T2Src.Y + CircleT1Radius, T1Src.Y + CircleT1Radius), RotOrigin.Y + CirclebaseRadius);
	auto xMin = std::min(std::min(T2Src.X - CircleT1Radius, T1Src.X - CircleT1Radius), RotOrigin.X - CirclebaseRadius);
	auto yMin = std::min(std::min(T2Src.Y - CircleT1Radius, T1Src.Y - CircleT1Radius), RotOrigin.Y - CirclebaseRadius);

	if (aabb)
	{
		aabb->Merge({xMax, yMax, xMin, yMin});
	}

	TTableLayer::edges_insert_square(yMin, xMin, yMax, xMax, this, nullptr);

	auto offset = 1.0f / InvT1Radius + pb::ball_min_smth;
	XMin = xMin - offset;
	YMin = yMin - offset;
	XMax = xMax + offset;
	YMax = yMax + offset;
}

void TFlipperEdge::set_control_points(float angle)
{
	float sin, cos;
	maths::SinCos(angle, sin, cos);
	A1 = A1Src;
	A2 = A2Src;
	B1 = B1Src;
	B2 = B2Src;
	T1 = T1Src;
	maths::RotatePt(A1, sin, cos, RotOrigin);
	maths::RotatePt(A2, sin, cos, RotOrigin);
	maths::RotatePt(T1, sin, cos, RotOrigin);
	maths::RotatePt(B1, sin, cos, RotOrigin);
	maths::RotatePt(B2, sin, cos, RotOrigin);
	maths::line_init(LineA, A1.X, A1.Y, A2.X, A2.Y);
	maths::line_init(LineB, B1.X, B1.Y, B2.X, B2.Y);
	circlebase = {RotOrigin, CirclebaseRadiusSq};
	circleT1 = {T1, CircleT1RadiusSq};
	ControlPointDirtyFlag = false;
}

float TFlipperEdge::flipper_angle_delta(float timeDelta)
{
	if (FlipperFlag == MessageCode::TFlipperNull)
		return 0.0f;

	const auto deltaAngle = MoveSpeed * timeDelta;
	if (std::fabs(deltaAngle) > AngleRemainder)
		return AngleDst - CurrentAngle;
	return deltaAngle;
}

int TFlipperEdge::SetMotion(MessageCode code)
{
	switch (code)
	{
	case MessageCode::TFlipperExtend:
		AngleRemainder = std::fabs(AngleMax - CurrentAngle);
		AngleDst = AngleMax;
		MoveSpeed = ExtendSpeed;
		break;
	case MessageCode::TFlipperRetract:
		AngleRemainder = std::fabs(CurrentAngle);
		AngleDst = 0.0f;
		MoveSpeed = RetractSpeed;
		break;
	case MessageCode::Reset:
		AngleRemainder = 0.0f;
		AngleDst = 0.0f;
		break;
	default: break;
	}

	if (AngleRemainder == 0.0f)
		code = MessageCode::TFlipperNull;

	FlipperFlag = code;
	return static_cast<int>(code);
}
