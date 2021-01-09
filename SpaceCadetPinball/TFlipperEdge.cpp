#include "pch.h"
#include "TFlipperEdge.h"


#include "TLine.h"
#include "TPinballTable.h"
#include "TTableLayer.h"

float TFlipperEdge::flipper_sin_angle, TFlipperEdge::flipper_cos_angle;
vector_type TFlipperEdge::A1, TFlipperEdge::A2, TFlipperEdge::B1, TFlipperEdge::B2, TFlipperEdge::T1;
line_type TFlipperEdge::lineA, TFlipperEdge::lineB;
circle_type TFlipperEdge::circlebase, TFlipperEdge::circleT1;

TFlipperEdge::TFlipperEdge(TCollisionComponent* collComp, char* someFlag, unsigned int visualFlag, TPinballTable* table,
                           vector_type* origin, vector_type* vecT, vector_type* vec3, float bmpCoef1, float bmpCoef2,
                           float a11, float c4F, float c5F): TEdgeSegment(collComp, someFlag, visualFlag)
{
	vector_type crossProd{}, vecDir1{}, vecDir2{};

	CollisionC4F = c4F;
	CollisionC5F = c5F;
	BmpCoef1 = bmpCoef1;
	BmpCoef2 = bmpCoef2;
	Unknown32F = a11;

	T1Src = *vecT;
	Unknown36V = *vec3;
	RotOrigin.X = origin->X;
	RotOrigin.Y = origin->Y;

	CirclebaseRadius = origin->Z + table->CollisionCompOffset;
	CirclebaseRadiusMSq = CirclebaseRadius * 1.01f * (CirclebaseRadius * 1.01f);
	CirclebaseRadiusSq = CirclebaseRadius * CirclebaseRadius;

	CircleT1Radius = vecT->Z + table->CollisionCompOffset;
	CircleT1RadiusMSq = CircleT1Radius * 1.01f * (CircleT1Radius * 1.01f);
	CircleT1RadiusSq = CircleT1Radius * CircleT1Radius;

	vecDir1.X = vecT->X - origin->X;
	vecDir1.Y = vecT->Y - origin->Y;
	vecDir1.Z = 0.0;
	maths::normalize_2d(&vecDir1);

	vecDir2.X = vec3->X - origin->X;
	vecDir2.Y = vec3->Y - origin->Y;
	vecDir2.Z = 0.0;
	maths::normalize_2d(&vecDir2);

	AngleMax = acos(maths::DotProduct(&vecDir1, &vecDir2));
	maths::cross(&vecDir1, &vecDir2, &crossProd);
	if (crossProd.Z < 0.0)
		AngleMax = -AngleMax;
	FlipperFlag = 0;
	Angle1 = 0.0;

	auto dirX1 = vecDir1.X;
	auto dirY1 = -vecDir1.Y;
	A2Src.X = dirY1 * CirclebaseRadius + origin->X;
	A2Src.Y = dirX1 * CirclebaseRadius + origin->Y;
	A1Src.X = dirY1 * CircleT1Radius + vecT->X;
	A1Src.Y = dirX1 * CircleT1Radius + vecT->Y;

	dirX1 = -dirX1;
	dirY1 = -dirY1;
	B1Src.X = dirY1 * CirclebaseRadius + origin->X;
	B1Src.Y = dirX1 * CirclebaseRadius + origin->Y;
	B2Src.X = dirY1 * CircleT1Radius + vecT->X;
	B2Src.Y = dirX1 * CircleT1Radius + vecT->Y;

	if (AngleMax < 0.0)
	{
		maths::vswap(&A1Src, &B1Src);
		maths::vswap(&A2Src, &B2Src);
	}

	auto dx = vecT->X - RotOrigin.X;
	auto dy = vecT->Y - RotOrigin.Y;
	auto distance1 = sqrt(dy * dy + dx * dx) + table->CollisionCompOffset + vecT->Z;
	DistanceDivSq = distance1 * distance1;

	float bmpCoef = min(BmpCoef1, BmpCoef2);
	auto distance = maths::Distance(vecT, vec3);
	Unknown40F = bmpCoef / (distance / CircleT1Radius + distance / CircleT1Radius);

	TFlipperEdge::place_in_grid();
	Unknown44 = 0;
	TimeAngle = 0.0;
	Unknown15 = 0;
	Unknown46F = 0.0;
	AngleMult = 0.0;
}

void TFlipperEdge::port_draw()
{
	set_control_points(TimeAngle);
	build_edges_in_motion();
}

float TFlipperEdge::FindCollisionDistance(ray_type* ray)
{
	auto ogRay = ray;


	float* pfVar2;
	short uVar3;
	int iVar4;
	int uVar5;
	vector_type* prVar6;
	vector_type* plVar6;
	vector_type* pvVar7;
	float fVar8;
	ray_type ray2;
	ray_type ray1;
	float local_1c;
	float local_18;
	float local_14;
	float local_10;
	float local_c;
	float local_8;

	if (ogRay->TimeNow > this->Unknown46F) {
		this->FlipperFlag = 0;
	}
	if (this->Unknown44 == 0) {
		if (this->FlipperFlag == 0) {
			this->Unknown44 = 0;
			this->Unknown15 = 0;
			this->Unknown16 = 0;
			set_control_points( ogRay->TimeNow);
			build_edges_in_motion();
			iVar4 = is_ball_inside( (ogRay->Origin).X, (ogRay->Origin).Y);
			ray1.MinDistance = ogRay->MinDistance;
			if (iVar4 == 0) {
				ray1.Direction.X = (ogRay->Direction).X;
				ray1.Direction.Y = (ogRay->Direction).Y;
				ray1.Direction.Z = (ogRay->Direction).Z;
				ray1.MaxDistance = ogRay->MaxDistance;
				ray1.Origin.X = (ogRay->Origin).X;
				ray1.Origin.Y = (ogRay->Origin).Y;
				ray1.Origin.Z = (ogRay->Origin).Z;
				fVar8 = maths::distance_to_flipper(&ray1, &ray2);
				plVar6 = &ray2.Origin;
				if (fVar8 == 0.0) {
					pvVar7 = &this->NextBallPosition;
					pvVar7->X = ray2.Origin.X;
					(this->NextBallPosition).Y = ray2.Origin.Y;
					(this->NextBallPosition).Z = ray2.Origin.Z;
					pvVar7->X = pvVar7->X - ray1.Direction.X * 1e-05f;
					pfVar2 = &(this->NextBallPosition).Y;
					*pfVar2 = *pfVar2 - ray1.Direction.Y * 1e-05f;
				}
				else {
					pvVar7 = &this->NextBallPosition;
				LAB_0101bab7:
					pvVar7->X = (plVar6)->X;
					pvVar7->Y = (plVar6)->Y;
					pvVar7->Z = (plVar6)->Z;
				}
				(this->CollisionDirection).X = ray2.Direction.X;
				(this->CollisionDirection).Y = ray2.Direction.Y;
				(this->CollisionDirection).Z = ray2.Direction.Z;
				return fVar8;
			}
			fVar8 = maths::Distance_Squared(ogRay->Origin, this->RotOrigin);
			if (this->CirclebaseRadiusMSq <= fVar8) {
				fVar8 = maths::Distance_Squared(ogRay->Origin, T1);
				if (this->CircleT1RadiusMSq <= fVar8) {
					ray1.Direction.Y = lineB.PerpendicularL.Y;
					ray1.Direction.X = lineB.PerpendicularL.X;
					if (iVar4 == 4) {
						ray1.Direction.Y = lineA.PerpendicularL.Y;
						ray1.Direction.X = lineA.PerpendicularL.X;
					}
					ray1.Direction.X = -ray1.Direction.X;
					ray1.Direction.Y = -ray1.Direction.Y;
				}
				else {
					ray1.Direction.X = T1.X - (ogRay->Origin).X;
					ray1.Direction.Y = T1.Y - (ogRay->Origin).Y;
					maths::normalize_2d(&ray1.Direction);
				}
			}
			else {
				ray1.Direction.X = (this->RotOrigin).X - (ogRay->Origin).X;
				ray1.Direction.Y = (this->RotOrigin).Y - (ogRay->Origin).Y;
				maths::normalize_2d(&ray1.Direction);
			}
			ray1.Origin.X = (ogRay->Origin).X - ray1.Direction.X * 5.0f;
			ray1.Origin.Y = (ogRay->Origin).Y - ray1.Direction.Y * 5.0f;
			ray1.MaxDistance = ogRay->MaxDistance + 10.0f;
			fVar8 = maths::distance_to_flipper(&ray1, &ray2);
			if (1e+09 <= fVar8) {
				ray1.Direction.X = (this->RotOrigin).X - (ogRay->Origin).X;
				ray1.Direction.Y = (this->RotOrigin).Y - (ogRay->Origin).Y;
				maths::normalize_2d(&ray1.Direction);
				ray1.Origin.X = (ogRay->Origin).X - ray1.Direction.X * 5.0f;
				ray1.Origin.Y = (ogRay->Origin).Y - ray1.Direction.Y * 5.0f;
				fVar8 = maths::distance_to_flipper(&ray1, &ray2);
				if (1e+09 <= fVar8) {
					return 1e+09;
				}
			}
		LAB_0101ba1a:
			(this->NextBallPosition).X = ray2.Origin.X;
			(this->NextBallPosition).Y = ray2.Origin.Y;
			(this->NextBallPosition).Z = ray2.Origin.Z;
			pvVar7 = &this->CollisionDirection;
			prVar6 = &ray2.Direction;
		LAB_0101bc82:
			pvVar7->X = prVar6->X;
			pvVar7->Y = prVar6->Y;
			pvVar7->Z = prVar6->Z;
			(this->NextBallPosition).X = (this->NextBallPosition).X - ray1.Direction.X * 1e-05;
			pfVar2 = &(this->NextBallPosition).Y;
			*pfVar2 = *pfVar2 - ray1.Direction.Y * 1e-05;
			return 0.0;
		}
		local_8 = (ogRay->Origin).X;
		local_14 = this->Unknown40F * ogRay->MaxDistance;
		local_c = (ogRay->Origin).Y;
		local_10 = ogRay->TimeNow;
		local_18 = this->Unknown40F * (ogRay->Direction).X;
		local_1c = (ogRay->Direction).Y * this->Unknown40F;
		fVar8 = ogRay->TimeDelta + ogRay->TimeNow;
		uVar3 = fVar8 <= local_10;// fp flag shift
		while (uVar3 == 0) {
			set_control_points( local_10);
			build_edges_in_motion();
			iVar4 = is_ball_inside( local_8, local_c);
			if (iVar4 != 0) {
				if ((this->FlipperFlag == 1) && (iVar4 != 5)) {
					plVar6 = &lineA.PerpendicularL;
					ray1.Direction.Y = lineA.PerpendicularL.Y;
					ray1.Direction.X = lineA.PerpendicularL.X;
				}
				else {
					if ((this->FlipperFlag != 2) || (iVar4 == 4)) {
						ray1.Direction.X = (this->RotOrigin).X - local_8;
						this->Unknown15 = 0;
						this->Unknown16 = 1;
						ray1.Direction.Y = (this->RotOrigin).Y - local_c;
						maths::normalize_2d(&ray1.Direction);
						ray1.Origin.X = local_8 - ray1.Direction.X * 5.0f;
						ray1.Origin.Y = local_c - ray1.Direction.Y * 5.0f;
						ray1.MaxDistance = ogRay->MaxDistance + 10.0f;
						fVar8 = maths::distance_to_flipper(&ray1, &ray2);
						if (1e+09 <= fVar8) {
							(this->NextBallPosition).X = local_8;
							(this->CollisionDirection).X = -ray1.Direction.X;
							(this->NextBallPosition).Y = local_c;
							(this->CollisionDirection).Y = -ray1.Direction.Y;
							return 0.0;
						}
						goto LAB_0101ba1a;
					}
					plVar6 = &lineB.PerpendicularL;
					ray1.Direction.Y = lineB.PerpendicularL.Y;
					ray1.Direction.X = lineB.PerpendicularL.X;
				}
				ray1.Direction.X = -ray1.Direction.X;
				ray1.Direction.Y = -ray1.Direction.Y;
				(this->Unknown17V).X = plVar6->X;
				(this->Unknown17V).Y = plVar6->Y;
				(this->Unknown17V).Z = plVar6->Z;
				this->Unknown16 = 0;
				this->Unknown15 = 1;
				ray1.MinDistance = 0.002;
				ray1.Origin.X = (ogRay->Origin).X - ray1.Direction.X * 5.0f;
				ray1.Origin.Y = (ogRay->Origin).Y - ray1.Direction.Y * 5.0f;
				ray1.MaxDistance = ogRay->MaxDistance + 10.0f;
				fVar8 = maths::distance_to_flipper(&ray1, &ray2);
				(this->CollisionDirection).X = ray2.Direction.X;
				(this->CollisionDirection).Y = ray2.Direction.Y;
				(this->CollisionDirection).Z = ray2.Direction.Z;
				if (1e+09 <= fVar8) {
					return 1e+09;
				}
				pvVar7 = &this->NextBallPosition;
				prVar6 = &ray2.Origin;
				goto LAB_0101bc82;
			}
			ray1.Direction.X = (ogRay->Direction).X;
			ray1.Direction.Y = (ogRay->Direction).Y;
			ray1.Direction.Z = (ogRay->Direction).Z;
			ray1.MinDistance = ogRay->MinDistance;
			ray1.Origin.X = (ogRay->Origin).X;
			ray1.Origin.Y = (ogRay->Origin).Y;
			ray1.Origin.Z = (ogRay->Origin).Z;
			ray1.MaxDistance = local_14;
			fVar8 = maths::distance_to_flipper(&ray1, &ray2);
			if (fVar8 < 1e+09) {
				pvVar7 = &this->NextBallPosition;
				pvVar7->X = ray2.Origin.X;
				(this->NextBallPosition).Y = ray2.Origin.Y;
				(this->NextBallPosition).Z = ray2.Origin.Z;
				pvVar7->X = pvVar7->X - ray1.Direction.X * 1e-05f;
				pfVar2 = &(this->NextBallPosition).Y;
				*pfVar2 = *pfVar2 - ray1.Direction.Y * 1e-05f;
				uVar5 = this->AngleMax > 0.0;
				pvVar7 = &this->Unknown17V;
				if (this->FlipperFlag == 2) {
					plVar6 = &lineB.PerpendicularL;
					this->Unknown15 = (uVar5 == 0);
				}
				else {
					this->Unknown15 = uVar5;
					plVar6 = &lineA.PerpendicularL;
				}
				goto LAB_0101bab7;
			}
			local_10 = local_10 + this->Unknown40F;
			local_8 = local_8 + local_18;
			local_c = local_c + local_1c;
			fVar8 = ogRay->TimeDelta + ogRay->TimeNow;
			uVar3 = fVar8 <= local_10;
		}
	}
	else {
		this->Unknown44 = 0;
	}
	return 1e+09;
}

void TFlipperEdge::EdgeCollision(TBall* ball, float coef)
{
	Unknown44 = 1;
	if (!FlipperFlag || !Unknown16 || Unknown15)
	{
		float collMult = 0.0;
		if (Unknown15)
		{
			float dx = NextBallPosition.X - RotOrigin.X;
			float dy = NextBallPosition.Y - RotOrigin.Y;
			float distance = dy * dy + dx * dx;
			if (circlebase.RadiusSq * 1.01 < distance)
			{
				float v11;
				float v20 = sqrt(distance / DistanceDivSq) * (fabs(AngleMax) / AngleMult);
				float dot1 = maths::DotProduct(&Unknown17V, &CollisionDirection);
				if (dot1 >= 0.0)
					v11 = dot1 * v20;
				else
					v11 = 0.0;
				collMult = v11 * Unknown32F;
			}
		}

		float maxSpeed = collMult <= 0.0 ? 1000000000.0f : -1.0f;
		maths::basic_collision(
			ball,
			&NextBallPosition,
			&CollisionDirection,
			CollisionC4F,
			CollisionC5F,
			maxSpeed,
			collMult);
		return;
	}

	float c4F;
	float dx = NextBallPosition.X - RotOrigin.X;
	float dy = NextBallPosition.Y - RotOrigin.Y;
	float distance = dy * dy + dx * dx;
	if (circlebase.RadiusSq * 1.01 < distance)
		c4F = (1.0f - sqrt(distance / DistanceDivSq)) * CollisionC4F;
	else
		c4F = CollisionC4F;
	maths::basic_collision(ball, &NextBallPosition, &CollisionDirection, c4F, CollisionC5F, 1000000000.0, 0.0);
}

void TFlipperEdge::place_in_grid()
{
	float x0 = RotOrigin.X - CirclebaseRadius;
	float y0 = RotOrigin.Y - CirclebaseRadius;
	float x1 = CirclebaseRadius + RotOrigin.X;

	float v1 = RotOrigin.Y + CirclebaseRadius;
	float v2 = T1Src.X - CircleT1Radius;
	if (v2 < x0)
		x0 = v2;

	float v3 = T1Src.Y - CircleT1Radius;
	if (v3 < y0)
		y0 = v3;

	float v4 = CircleT1Radius + T1Src.X;
	if (v4 > x1)
		x1 = v4;

	float v5 = T1Src.Y + CircleT1Radius;
	if (v5 > v1)
		v1 = v5;

	float v6 = Unknown36V.X - CircleT1Radius;
	if (v6 < x0)
		x0 = v6;

	float v7 = Unknown36V.Y - CircleT1Radius;
	if (v7 < y0)
		y0 = v7;

	float v8 = Unknown36V.X + CircleT1Radius;
	if (v8 > x1)
		x1 = v8;

	float v9 = CircleT1Radius + Unknown36V.Y;
	if (v9 > v1)
		v1 = v9;

	float y1 = v1;
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
	if (angle < 0.0)
		angle = -angle;

	if (angle >= 0.0000001)
		angle = (timeNow - TimeAngle) / angle;
	else
		angle = 1.0;

	angle = min(1, max(angle, 0));
	if (FlipperFlag == 2)
		angle = 1.0f - angle;
	return angle * AngleMax;
}

int TFlipperEdge::is_ball_inside(float x, float y)
{
	vector_type ptTest{};
	float dx = RotOrigin.X - x;
	float dy = RotOrigin.Y - y;
	if ((A2.X - A1.X) * (y - A1.Y) - (A2.Y - A1.Y) * (x - A1.X) >= 0.0 &&
		(B1.X - A2.X) * (y - A2.Y) - (B1.Y - A2.Y) * (x - A2.X) >= 0.0 &&
		(B2.X - B1.X) * (y - B1.Y) - (B2.Y - B1.Y) * (x - B1.X) >= 0.0 &&
		(A1.X - B2.X) * (y - B2.Y) - (A1.Y - B2.Y) * (x - B2.X) >= 0.0 ||
		dy * dy + dx * dx <= CirclebaseRadiusSq ||
		(T1.Y - y) * (T1.Y - y) + (T1.X - x) * (T1.X - x) < CircleT1RadiusSq)
	{
		if (FlipperFlag == 1)
			ptTest = AngleMax < 0.0 ? B1 : B2;
		else if (FlipperFlag == 2)
			ptTest = AngleMax < 0.0 ? A2 : A1;
		else
			ptTest = T1;

		if ((y - ptTest.Y) * (RotOrigin.X - ptTest.X) -
			(x - ptTest.X) * (RotOrigin.Y - ptTest.Y) >= 0.0f)
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
		this->Angle2 = flipper_angle(value);
		this->Angle1 = this->AngleMax;
		this->AngleMult = this->BmpCoef1;
		break;
	case 2:
		this->Angle2 = flipper_angle(value);
		this->Angle1 = 0.0;
		this->AngleMult = this->BmpCoef2;
		break;
	case 1024:
		this->FlipperFlag = 0;
		this->Angle1 = 0.0;
		return;
	default: break;
	}

	if (!this->FlipperFlag)
		this->TimeAngle = value;
	this->FlipperFlag = code;
	this->Unknown46F = this->AngleMult + this->TimeAngle;
}
