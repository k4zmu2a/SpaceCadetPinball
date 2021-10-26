#include "pch.h"
#include "TFlipperEdge.h"


#include "TLine.h"
#include "TPinballTable.h"
#include "TTableLayer.h"

float TFlipperEdge::flipper_sin_angle, TFlipperEdge::flipper_cos_angle;
vector_type TFlipperEdge::A1, TFlipperEdge::A2, TFlipperEdge::B1, TFlipperEdge::B2, TFlipperEdge::T1;
line_type TFlipperEdge::lineA, TFlipperEdge::lineB;
circle_type TFlipperEdge::circlebase, TFlipperEdge::circleT1;

TFlipperEdge::TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup, TPinballTable* table,
                           vector_type* origin, vector_type* vecT1, vector_type* vecT2, float extendTime, float retractTime,
                           float collMult, float elasticity, float smoothness): TEdgeSegment(collComp, activeFlag, collisionGroup)
{
	vector_type crossProd{}, vecDir1{}, vecDir2{};

	Elasticity = elasticity;
	Smoothness = smoothness;
	ExtendTime = extendTime;
	RetractTime = retractTime;
	CollisionMult = collMult;

	T1Src = *vecT1;
	T2Src = *vecT2;
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
	maths::normalize_2d(&vecDir1);

	vecDir2.X = vecT2->X - origin->X;
	vecDir2.Y = vecT2->Y - origin->Y;
	vecDir2.Z = 0.0;
	maths::normalize_2d(&vecDir2);

	AngleMax = acos(maths::DotProduct(&vecDir1, &vecDir2));
	maths::cross(&vecDir1, &vecDir2, &crossProd);
	if (crossProd.Z < 0.0f)
		AngleMax = -AngleMax;
	FlipperFlag = 0;
	Angle1 = 0.0;

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
		maths::vswap(&A1Src, &B1Src);
		maths::vswap(&A2Src, &B2Src);
	}

	auto dx = vecT1->X - RotOrigin.X;
	auto dy = vecT1->Y - RotOrigin.Y;
	auto distance1 = sqrt(dy * dy + dx * dx) + table->CollisionCompOffset + vecT1->Z;
	DistanceDivSq = distance1 * distance1;

	float minMoveTime = std::min(ExtendTime, RetractTime);
	auto distance = maths::Distance(vecT1, vecT2);
	CollisionTimeAdvance = minMoveTime / (distance / CircleT1Radius + distance / CircleT1Radius);

	TFlipperEdge::place_in_grid();
	EdgeCollisionFlag = 0;
	InputTime = 0.0;
	CollisionFlag1 = 0;
	AngleStopTime = 0.0;
	AngleMult = 0.0;
}

void TFlipperEdge::port_draw()
{
	set_control_points(InputTime);
	build_edges_in_motion();
}

float TFlipperEdge::FindCollisionDistance(ray_type* ray)
{
	auto ogRay = ray;
	ray_type dstRay{}, srcRay{};

	if (ogRay->TimeNow > AngleStopTime)
	{
		FlipperFlag = 0;
	}
	if (EdgeCollisionFlag == 0)
	{
		if (FlipperFlag == 0)
		{
			CollisionFlag1 = 0;
			CollisionFlag2 = 0;
			set_control_points(ogRay->TimeNow);
			build_edges_in_motion();
			auto ballInside = is_ball_inside(ogRay->Origin.X, ogRay->Origin.Y);
			srcRay.MinDistance = ogRay->MinDistance;
			if (ballInside == 0)
			{
				srcRay.Direction = ogRay->Direction;
				srcRay.MaxDistance = ogRay->MaxDistance;
				srcRay.Origin = ogRay->Origin;
				auto distance = maths::distance_to_flipper(&srcRay, &dstRay);
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
					srcRay.Direction.Y = lineB.PerpendicularL.Y;
					srcRay.Direction.X = lineB.PerpendicularL.X;
					if (ballInside == 4)
					{
						srcRay.Direction.Y = lineA.PerpendicularL.Y;
						srcRay.Direction.X = lineA.PerpendicularL.X;
					}
					srcRay.Direction.X = -srcRay.Direction.X;
					srcRay.Direction.Y = -srcRay.Direction.Y;
				}
				else
				{
					srcRay.Direction.X = T1.X - ogRay->Origin.X;
					srcRay.Direction.Y = T1.Y - ogRay->Origin.Y;
					maths::normalize_2d(&srcRay.Direction);
				}
			}
			else
			{
				srcRay.Direction.X = RotOrigin.X - ogRay->Origin.X;
				srcRay.Direction.Y = RotOrigin.Y - ogRay->Origin.Y;
				maths::normalize_2d(&srcRay.Direction);
			}

			srcRay.Origin.X = ogRay->Origin.X - srcRay.Direction.X * 5.0f;
			srcRay.Origin.Y = ogRay->Origin.Y - srcRay.Direction.Y * 5.0f;
			srcRay.MaxDistance = ogRay->MaxDistance + 10.0f;
			if (maths::distance_to_flipper(&srcRay, &dstRay) >= 1e+09f)
			{
				srcRay.Direction.X = RotOrigin.X - ogRay->Origin.X;
				srcRay.Direction.Y = RotOrigin.Y - ogRay->Origin.Y;
				maths::normalize_2d(&srcRay.Direction);
				srcRay.Origin.X = ogRay->Origin.X - srcRay.Direction.X * 5.0f;
				srcRay.Origin.Y = ogRay->Origin.Y - srcRay.Direction.Y * 5.0f;
				if (maths::distance_to_flipper(&srcRay, &dstRay) >= 1e+09f)
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
			build_edges_in_motion();
			auto ballInside = is_ball_inside(posX, posY);
			if (ballInside != 0)
			{
				vector_type* linePtr;
				if (FlipperFlag == 1 && ballInside != 5)
				{
					linePtr = &lineA.PerpendicularL;
					srcRay.Direction.Y = lineA.PerpendicularL.Y;
					srcRay.Direction.X = lineA.PerpendicularL.X;
				}
				else
				{
					if (FlipperFlag != 2 || ballInside == 4)
					{
						CollisionFlag1 = 0;
						CollisionFlag2 = 1;
						srcRay.Direction.X = RotOrigin.X - posX;
						srcRay.Direction.Y = RotOrigin.Y - posY;
						maths::normalize_2d(&srcRay.Direction);

						srcRay.Origin.X = posX - srcRay.Direction.X * 5.0f;
						srcRay.Origin.Y = posY - srcRay.Direction.Y * 5.0f;
						srcRay.MaxDistance = ogRay->MaxDistance + 10.0f;
						if (maths::distance_to_flipper(&srcRay, &dstRay) >= 1e+09f)
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
					linePtr = &lineB.PerpendicularL;
					srcRay.Direction.Y = lineB.PerpendicularL.Y;
					srcRay.Direction.X = lineB.PerpendicularL.X;
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
				auto distance = maths::distance_to_flipper(&srcRay, &dstRay);
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
			auto distance = maths::distance_to_flipper(&srcRay, &dstRay);
			if (distance < 1e+09f)
			{
				NextBallPosition = dstRay.Origin;
				NextBallPosition.X -= srcRay.Direction.X * 1e-05f;
				NextBallPosition.Y -= srcRay.Direction.Y * 1e-05f;
				vector_type* linePtr;
				if (FlipperFlag == 2)
				{
					linePtr = &lineB.PerpendicularL;
					CollisionFlag1 = AngleMax <= 0.0f;
				}
				else
				{
					CollisionFlag1 = AngleMax > 0.0f;
					linePtr = &lineA.PerpendicularL;
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

void TFlipperEdge::EdgeCollision(TBall* ball, float coef)
{
	EdgeCollisionFlag = 1;
	if (!FlipperFlag || !CollisionFlag2 || CollisionFlag1)
	{
		float boost = 0.0;
		if (CollisionFlag1)
		{
			float dx = NextBallPosition.X - RotOrigin.X;
			float dy = NextBallPosition.Y - RotOrigin.Y;
			float distance = dy * dy + dx * dx;
			if (circlebase.RadiusSq * 1.01f < distance)
			{
				float v11;
				float v20 = sqrt(distance / DistanceDivSq) * (fabs(AngleMax) / AngleMult);
				float dot1 = maths::DotProduct(&CollisionLinePerp, &CollisionDirection);
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
	float distance = dy * dy + dx * dx;
	if (circlebase.RadiusSq * 1.01f < distance)
		elasticity = (1.0f - sqrt(distance / DistanceDivSq)) * Elasticity;
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
	maths::SinCos(flipper_angle(timeNow), &flipper_sin_angle, &flipper_cos_angle);
	A1 = A1Src;
	A2 = A2Src;
	B1 = B1Src;
	B2 = B2Src;
	T1 = T1Src;
	maths::RotatePt(&A1, flipper_sin_angle, flipper_cos_angle, &RotOrigin);
	maths::RotatePt(&A2, flipper_sin_angle, flipper_cos_angle, &RotOrigin);
	maths::RotatePt(&T1, flipper_sin_angle, flipper_cos_angle, &RotOrigin);
	maths::RotatePt(&B1, flipper_sin_angle, flipper_cos_angle, &RotOrigin);
	maths::RotatePt(&B2, flipper_sin_angle, flipper_cos_angle, &RotOrigin);
}

void TFlipperEdge::build_edges_in_motion()
{
	maths::line_init(&lineA, A1.X, A1.Y, A2.X, A2.Y);
	maths::line_init(&lineB, B1.X, B1.Y, B2.X, B2.Y);
	circlebase.RadiusSq = CirclebaseRadiusSq;
	circlebase.Center.X = RotOrigin.X;
	circlebase.Center.Y = RotOrigin.Y;
	circleT1.RadiusSq = CircleT1RadiusSq;
	circleT1.Center.X = T1.X;
	circleT1.Center.Y = T1.Y;
}

float TFlipperEdge::flipper_angle(float timeNow)
{
	if (!FlipperFlag)
		return Angle1;
	float angle = (Angle1 - Angle2) / AngleMax * AngleMult;
	if (angle < 0.0f)
		angle = -angle;

	if (angle >= 0.0000001f)
		angle = (timeNow - InputTime) / angle;
	else
		angle = 1.0;

	angle = std::min(1.0f, std::max(angle, 0.0f));
	if (FlipperFlag == 2)
		angle = 1.0f - angle;
	return angle * AngleMax;
}

int TFlipperEdge::is_ball_inside(float x, float y)
{
	vector_type testPoint{};
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
		if (FlipperFlag == 1)
			testPoint = AngleMax < 0.0f ? B1 : B2;
		else if (FlipperFlag == 2)
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

void TFlipperEdge::SetMotion(int code, float value)
{
	switch (code)
	{
	case 1:
		Angle2 = flipper_angle(value);
		Angle1 = AngleMax;
		AngleMult = ExtendTime;
		break;
	case 2:
		Angle2 = flipper_angle(value);
		Angle1 = 0.0;
		AngleMult = RetractTime;
		break;
	case 1024:
		FlipperFlag = 0;
		Angle1 = 0.0;
		return;
	default: break;
	}

	if (!FlipperFlag)
		InputTime = value;
	FlipperFlag = code;
	AngleStopTime = AngleMult + InputTime;
}
