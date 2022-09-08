#include "pch.h"
#include "TFlipperEdge.h"


#include "pb.h"
#include "TLine.h"
#include "TPinballTable.h"
#include "TTableLayer.h"


TFlipperEdge::TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup, TPinballTable* table,
                           vector3* origin, vector3* vecT1, vector3* vecT2, float extendTime, float retractTime,
                           float collMult, float elasticity, float smoothness): TEdgeSegment(collComp, activeFlag, collisionGroup)
{
	vector3 crossProd{}, vecDir1{}, vecDir2{};

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

	vecDir1.X = vecT1->X - origin->X;
	vecDir1.Y = vecT1->Y - origin->Y;
	vecDir1.Z = 0.0;
	maths::normalize_2d(vecDir1);

	vecDir2.X = vecT2->X - origin->X;
	vecDir2.Y = vecT2->Y - origin->Y;
	vecDir2.Z = 0.0;
	maths::normalize_2d(vecDir2);

	AngleMax = acos(maths::DotProduct(vecDir1, vecDir2));
	maths::cross(vecDir1, vecDir2, crossProd);
	if (crossProd.Z < 0.0f)
		AngleMax = -AngleMax;
	FlipperFlag = MessageCode::TFlipperNull;
	AngleDst = 0.0;

	// 3DPB and FT have different formats for flipper speed:
	// 3DPB: Time it takes for flipper to go from source to destination, in sec.
	// FT: Flipper movement speed, in radians per sec.
	if (pb::FullTiltMode)
	{
		auto angleMax = std::abs(AngleMax);
		retractTime = angleMax / retractTime;
		extendTime = angleMax / extendTime;
	}
	ExtendTime = extendTime;
	RetractTime = retractTime;

	auto dirX1 = vecDir1.X;
	auto dirY1 = -vecDir1.Y;
	A2Src.X = dirY1 * CirclebaseRadius + origin->X;
	A2Src.Y = dirX1 * CirclebaseRadius + origin->Y;
	A1Src.X = dirY1 * CircleT1Radius + vecT1->X;
	A1Src.Y = dirX1 * CircleT1Radius + vecT1->Y;

	dirX1 = -dirX1;
	dirY1 = -dirY1;
	B1Src.X = dirY1 * CirclebaseRadius + origin->X;
	B1Src.Y = dirX1 * CirclebaseRadius + origin->Y;
	B2Src.X = dirY1 * CircleT1Radius + vecT1->X;
	B2Src.Y = dirX1 * CircleT1Radius + vecT1->Y;

	if (AngleMax < 0.0f)
	{
		std::swap(A1Src, B1Src);
		std::swap(A2Src, B2Src);
	}

	auto dx = vecT1->X - RotOrigin.X;
	auto dy = vecT1->Y - RotOrigin.Y;
	auto distance1 = sqrt(dy * dy + dx * dx) + table->CollisionCompOffset + vecT1->Z;
	DistanceDivSq = distance1 * distance1;

	float minMoveTime = std::min(ExtendTime, RetractTime);
	auto distance = maths::Distance(*vecT1, *vecT2);
	CollisionTimeAdvance = minMoveTime / (distance / CircleT1Radius + distance / CircleT1Radius);

	TFlipperEdge::place_in_grid();
	EdgeCollisionFlag = 0;
	InputTime = 0.0;
	CollisionFlag1 = 0;
	AngleStopTime = 0.0;
	AngleAdvanceTime = 0.0;
}

void TFlipperEdge::port_draw()
{
	set_control_points(InputTime);
}

float TFlipperEdge::FindCollisionDistance(ray_type* ray)
{
	auto ogRay = ray;
	ray_type dstRay{}, srcRay{};

	if (ogRay->TimeNow > AngleStopTime)
	{
		FlipperFlag = MessageCode::TFlipperNull;
	}
	if (EdgeCollisionFlag == 0)
	{
		if (FlipperFlag == MessageCode::TFlipperNull)
		{
			CollisionFlag1 = 0;
			CollisionFlag2 = 0;
			set_control_points(ogRay->TimeNow);
			auto ballInside = is_ball_inside(ogRay->Origin.X, ogRay->Origin.Y);
			srcRay.MinDistance = ogRay->MinDistance;
			if (ballInside == 0)
			{
				srcRay.Direction = ogRay->Direction;
				srcRay.MaxDistance = ogRay->MaxDistance;
				srcRay.Origin = ogRay->Origin;
				auto distance = maths::distance_to_flipper(this, srcRay, dstRay);
				if (distance == 0.0f)
				{
					NextBallPosition = dstRay.Origin;
					NextBallPosition.X -= srcRay.Direction.X * 1e-05f;
					NextBallPosition.Y -= srcRay.Direction.Y * 1e-05f;
				}
				else
				{
					NextBallPosition = dstRay.Origin;
				}
				CollisionDirection = dstRay.Direction;
				return distance;
			}

			if (maths::Distance_Squared(ogRay->Origin, RotOrigin) >= CirclebaseRadiusMSq)
			{
				if (maths::Distance_Squared(ogRay->Origin, T1) >= CircleT1RadiusMSq)
				{
					srcRay.Direction.Y = lineB.PerpendicularC.Y;
					srcRay.Direction.X = lineB.PerpendicularC.X;
					if (ballInside == 4)
					{
						srcRay.Direction.Y = lineA.PerpendicularC.Y;
						srcRay.Direction.X = lineA.PerpendicularC.X;
					}
					srcRay.Direction.X = -srcRay.Direction.X;
					srcRay.Direction.Y = -srcRay.Direction.Y;
				}
				else
				{
					srcRay.Direction.X = T1.X - ogRay->Origin.X;
					srcRay.Direction.Y = T1.Y - ogRay->Origin.Y;
					maths::normalize_2d(srcRay.Direction);
				}
			}
			else
			{
				srcRay.Direction.X = RotOrigin.X - ogRay->Origin.X;
				srcRay.Direction.Y = RotOrigin.Y - ogRay->Origin.Y;
				maths::normalize_2d(srcRay.Direction);
			}

			srcRay.Origin.X = ogRay->Origin.X - srcRay.Direction.X * 5.0f;
			srcRay.Origin.Y = ogRay->Origin.Y - srcRay.Direction.Y * 5.0f;
			srcRay.MaxDistance = ogRay->MaxDistance + 10.0f;
			if (maths::distance_to_flipper(this, srcRay, dstRay) >= 1e+09f)
			{
				srcRay.Direction.X = RotOrigin.X - ogRay->Origin.X;
				srcRay.Direction.Y = RotOrigin.Y - ogRay->Origin.Y;
				maths::normalize_2d(srcRay.Direction);
				srcRay.Origin.X = ogRay->Origin.X - srcRay.Direction.X * 5.0f;
				srcRay.Origin.Y = ogRay->Origin.Y - srcRay.Direction.Y * 5.0f;
				if (maths::distance_to_flipper(this, srcRay, dstRay) >= 1e+09f)
				{
					return 1e+09;
				}
			}

			NextBallPosition = dstRay.Origin;
			CollisionDirection = dstRay.Direction;
			NextBallPosition.X -= srcRay.Direction.X * 1e-05f;
			NextBallPosition.Y -= srcRay.Direction.Y * 1e-05f;
			return 0.0;
		}

		auto posX = ogRay->Origin.X;
		auto posY = ogRay->Origin.Y;
		auto posXAdvance = ogRay->Direction.X * CollisionTimeAdvance;
		auto posYAdvance = ogRay->Direction.Y * CollisionTimeAdvance;
		auto rayMaxDistance = ogRay->MaxDistance * CollisionTimeAdvance;
		auto timeNow = ogRay->TimeNow;
		auto stopTime = ogRay->TimeDelta + ogRay->TimeNow;
		while (timeNow < stopTime)
		{
			set_control_points(timeNow);
			auto ballInside = is_ball_inside(posX, posY);
			if (ballInside != 0)
			{
				vector2* linePtr;
				if (FlipperFlag == MessageCode::TFlipperExtend && ballInside != 5)
				{
					linePtr = &lineA.PerpendicularC;
					srcRay.Direction.Y = lineA.PerpendicularC.Y;
					srcRay.Direction.X = lineA.PerpendicularC.X;
				}
				else
				{
					if (FlipperFlag != MessageCode::TFlipperRetract || ballInside == 4)
					{
						CollisionFlag1 = 0;
						CollisionFlag2 = 1;
						srcRay.Direction.X = RotOrigin.X - posX;
						srcRay.Direction.Y = RotOrigin.Y - posY;
						maths::normalize_2d(srcRay.Direction);

						srcRay.Origin.X = posX - srcRay.Direction.X * 5.0f;
						srcRay.Origin.Y = posY - srcRay.Direction.Y * 5.0f;
						srcRay.MaxDistance = ogRay->MaxDistance + 10.0f;
						if (maths::distance_to_flipper(this, srcRay, dstRay) >= 1e+09f)
						{
							NextBallPosition.X = posX;
							NextBallPosition.Y = posY;
							CollisionDirection.X = -srcRay.Direction.X;
							CollisionDirection.Y = -srcRay.Direction.Y;
							return 0.0;
						}

						NextBallPosition = dstRay.Origin;
						CollisionDirection = dstRay.Direction;
						NextBallPosition.X -= srcRay.Direction.X * 1e-05f;
						NextBallPosition.Y -= srcRay.Direction.Y * 1e-05f;
						return 0.0;
					}
					linePtr = &lineB.PerpendicularC;
					srcRay.Direction.Y = lineB.PerpendicularC.Y;
					srcRay.Direction.X = lineB.PerpendicularC.X;
				}

				CollisionLinePerp = *linePtr;
				CollisionFlag2 = 0;
				CollisionFlag1 = 1;
				srcRay.Direction.X = -srcRay.Direction.X;
				srcRay.Direction.Y = -srcRay.Direction.Y;
				srcRay.MinDistance = 0.002f;
				srcRay.Origin.X = ogRay->Origin.X - srcRay.Direction.X * 5.0f;
				srcRay.Origin.Y = ogRay->Origin.Y - srcRay.Direction.Y * 5.0f;
				srcRay.MaxDistance = ogRay->MaxDistance + 10.0f;
				auto distance = maths::distance_to_flipper(this, srcRay, dstRay);
				CollisionDirection = dstRay.Direction;
				if (distance >= 1e+09f)
				{
					return 1e+09;
				}
				NextBallPosition = dstRay.Origin;
				NextBallPosition.X -= srcRay.Direction.X * 1e-05f;
				NextBallPosition.Y -= srcRay.Direction.Y * 1e-05f;
				return 0.0;
			}

			srcRay.Direction = ogRay->Direction;
			srcRay.MinDistance = ogRay->MinDistance;
			srcRay.Origin = ogRay->Origin;
			srcRay.MaxDistance = rayMaxDistance;
			auto distance = maths::distance_to_flipper(this, srcRay, dstRay);
			if (distance < 1e+09f)
			{
				NextBallPosition = dstRay.Origin;
				NextBallPosition.X -= srcRay.Direction.X * 1e-05f;
				NextBallPosition.Y -= srcRay.Direction.Y * 1e-05f;
				vector2* linePtr;
				if (FlipperFlag == MessageCode::TFlipperRetract)
				{
					linePtr = &lineB.PerpendicularC;
					CollisionFlag1 = AngleMax <= 0.0f;
				}
				else
				{
					CollisionFlag1 = AngleMax > 0.0f;
					linePtr = &lineA.PerpendicularC;
				}
				CollisionLinePerp = *linePtr;
				CollisionDirection = dstRay.Direction;
				return distance;
			}
			timeNow = timeNow + CollisionTimeAdvance;
			posX = posX + posXAdvance;
			posY = posY + posYAdvance;
		}
	}
	else
	{
		EdgeCollisionFlag = 0;
	}
	return 1e+09;
}

void TFlipperEdge::EdgeCollision(TBall* ball, float distance)
{
	EdgeCollisionFlag = 1;
	if (FlipperFlag == MessageCode::TFlipperNull || !CollisionFlag2 || CollisionFlag1)
	{
		float boost = 0.0;
		if (CollisionFlag1)
		{
			float dx = NextBallPosition.X - RotOrigin.X;
			float dy = NextBallPosition.Y - RotOrigin.Y;
			float distanceSq = dy * dy + dx * dx;
			if (circlebase.RadiusSq * 1.01f < distanceSq)
			{
				float v11;
				float v20 = sqrt(distanceSq / DistanceDivSq) * (fabs(AngleMax) / AngleAdvanceTime);
				float dot1 = maths::DotProduct(CollisionLinePerp, CollisionDirection);
				if (dot1 >= 0.0f)
					v11 = dot1 * v20;
				else
					v11 = 0.0;
				boost = v11 * CollisionMult;
			}
		}

		float threshold = boost <= 0.0f ? 1000000000.0f : -1.0f;
		maths::basic_collision(
			ball,
			&NextBallPosition,
			&CollisionDirection,
			Elasticity,
			Smoothness,
			threshold,
			boost);
		return;
	}

	float elasticity;
	float dx = NextBallPosition.X - RotOrigin.X;
	float dy = NextBallPosition.Y - RotOrigin.Y;
	float distanceSq = dy * dy + dx * dx;
	if (circlebase.RadiusSq * 1.01f < distanceSq)
		elasticity = (1.0f - sqrt(distanceSq / DistanceDivSq)) * Elasticity;
	else
		elasticity = Elasticity;
	maths::basic_collision(ball, &NextBallPosition, &CollisionDirection, elasticity, Smoothness, 1000000000.0, 0.0);
}

void TFlipperEdge::place_in_grid()
{
	float x0 = RotOrigin.X - CirclebaseRadius;
	float y0 = RotOrigin.Y - CirclebaseRadius;
	float x1 = RotOrigin.X + CirclebaseRadius;
	float y1 = RotOrigin.Y + CirclebaseRadius;

	float v2 = T1Src.X - CircleT1Radius;
	if (v2 < x0)
		x0 = v2;

	float v3 = T1Src.Y - CircleT1Radius;
	if (v3 < y0)
		y0 = v3;

	float v4 = T1Src.X + CircleT1Radius;
	if (v4 > x1)
		x1 = v4;

	float v5 = T1Src.Y + CircleT1Radius;
	if (v5 > y1)
		y1 = v5;

	float v6 = T2Src.X - CircleT1Radius;
	if (v6 < x0)
		x0 = v6;

	float v7 = T2Src.Y - CircleT1Radius;
	if (v7 < y0)
		y0 = v7;

	float v8 = T2Src.X + CircleT1Radius;
	if (v8 > x1)
		x1 = v8;

	float v9 = T2Src.Y + CircleT1Radius;
	if (v9 > y1)
		y1 = v9;

	TTableLayer::edges_insert_square(y0, x0, y1, x1, this, nullptr);
}

void TFlipperEdge::set_control_points(float timeNow)
{
	float sin, cos;
	maths::SinCos(flipper_angle(timeNow), sin, cos);
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
	maths::line_init(lineA, A1.X, A1.Y, A2.X, A2.Y);
	maths::line_init(lineB, B1.X, B1.Y, B2.X, B2.Y);
	circlebase = {RotOrigin, CirclebaseRadiusSq};
	circleT1 = {T1, CircleT1RadiusSq};
}

float TFlipperEdge::flipper_angle(float timeNow)
{
	// When not moving, flipper is at destination angle.
	if (FlipperFlag == MessageCode::TFlipperNull)
		return AngleDst;

	// How much time it takes to go from source to destination angle, in sec.
	auto arcDuration = std::abs((AngleDst - AngleSrc) / AngleMax * AngleAdvanceTime);

	// How close the flipper is to destination, in [0, 1] range.
	auto t = arcDuration >= 0.0000001f ? (timeNow - InputTime) / arcDuration : 1.0f;
	t = Clamp(t, 0.0f, 1.0f);

	// Result = linear interpolation between source and destination angle.
	return AngleSrc + t * (AngleDst - AngleSrc);
}

int TFlipperEdge::is_ball_inside(float x, float y)
{
	vector2 testPoint{};
	float dx = RotOrigin.X - x;
	float dy = RotOrigin.Y - y;
	if (((A2.X - A1.X) * (y - A1.Y) - (A2.Y - A1.Y) * (x - A1.X) >= 0.0f &&
		(B1.X - A2.X) * (y - A2.Y) - (B1.Y - A2.Y) * (x - A2.X) >= 0.0f &&
		(B2.X - B1.X) * (y - B1.Y) - (B2.Y - B1.Y) * (x - B1.X) >= 0.0f &&
		(A1.X - B2.X) * (y - B2.Y) - (A1.Y - B2.Y) * (x - B2.X) >= 0.0f) ||
		dy * dy + dx * dx <= CirclebaseRadiusSq ||
		(T1.Y - y) * (T1.Y - y) + (T1.X - x) * (T1.X - x) < CircleT1RadiusSq)
	{
		float flipperLR = AngleMax < 0.0f ? -1.0f : 1.0f;
		if (FlipperFlag == MessageCode::TFlipperExtend)
			testPoint = AngleMax < 0.0f ? B1 : B2;
		else if (FlipperFlag == MessageCode::TFlipperRetract)
			testPoint = AngleMax < 0.0f ? A2 : A1;
		else
			testPoint = T1;

		if (((y - testPoint.Y) * (RotOrigin.X - testPoint.X) -
			(x - testPoint.X) * (RotOrigin.Y - testPoint.Y)) * flipperLR < 0.0f)
			return 4;
		return 5;
	}
	return 0;
}

int TFlipperEdge::SetMotion(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TFlipperExtend:
		AngleSrc = flipper_angle(value);
		AngleDst = AngleMax;
		AngleAdvanceTime = ExtendTime;
		break;
	case MessageCode::TFlipperRetract:
		AngleSrc = flipper_angle(value);
		AngleDst = 0.0f;
		AngleAdvanceTime = RetractTime;
		break;
	case MessageCode::Reset:
		AngleSrc = 0.0f;
		AngleDst = 0.0f;
		break;
	default: break;
	}

	if (AngleSrc == AngleDst)
		code = MessageCode::TFlipperNull;

	InputTime = value;
	FlipperFlag = code;
	AngleStopTime = AngleAdvanceTime + InputTime;
	return static_cast<int>(code);
}
