#include "pch.h"
#include "maths.h"

#include "TBall.h"
#include "TFlipperEdge.h"


void maths::enclosing_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect)
{
	int xPos1 = rect1->XPosition;
	int yPos1 = rect1->YPosition;
	int width1 = rect1->Width;
	int height1 = rect1->Height;
	int xPos2 = rect2->XPosition;
	bool rect2XPosLessRect1 = rect2->XPosition < rect1->XPosition;
	int yPos2 = rect2->YPosition;
	int width2 = rect2->Width;
	int height2 = rect2->Height;
	int xPos2_2 = rect2->XPosition;
	if (rect2XPosLessRect1)
	{
		width1 += xPos1 - xPos2;
		xPos1 = xPos2;
	}
	if (yPos2 < yPos1)
	{
		height1 += yPos1 - yPos2;
		yPos1 = yPos2;
	}
	if (width2 + xPos2 > xPos1 + width1)
		width1 = xPos2_2 + width2 - xPos1;
	int height1_2 = height1;
	if (height2 + yPos2 > height1 + yPos1)
		height1_2 = yPos2 + height2 - yPos1;
	dstRect->YPosition = yPos1;
	dstRect->Height = height1_2;
	dstRect->XPosition = xPos1;
	dstRect->Width = width1;
}


int maths::rectangle_clip(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect)
{
	int xPos1 = rect1->XPosition;
	int yPos1 = rect1->YPosition;
	int height1 = rect1->Height;
	int xRight2 = rect2->XPosition + rect2->Width;
	int width1 = rect1->Width;
	int yRight2 = rect2->YPosition + rect2->Height;
	if (xPos1 + width1 < rect2->XPosition)
		return 0;
	if (xPos1 >= xRight2)
		return 0;
	int yPos2 = yPos1;
	if (yPos1 + height1 < rect2->YPosition || yPos1 >= yRight2)
		return 0;
	if (xPos1 < rect2->XPosition)
	{
		width1 += xPos1 - rect2->XPosition;
		xPos1 = rect2->XPosition;
	}
	if (xPos1 + width1 > xRight2)
		width1 = xRight2 - xPos1;
	int height2 = height1;
	if (yPos1 < rect2->YPosition)
	{
		height2 = yPos1 - rect2->YPosition + height1;
		yPos2 = rect2->YPosition;
	}
	if (height2 + yPos2 > yRight2)
		height2 = yRight2 - yPos2;
	if (!width1 || !height2)
		return 0;
	if (dstRect)
	{
		dstRect->XPosition = xPos1;
		dstRect->YPosition = yPos2;
		dstRect->Width = width1;
		dstRect->Height = height2;
	}
	return 1;
}


int maths::overlapping_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect)
{
	int v3;
	int v4;
	int v6;
	int v7;

	if (rect1->XPosition >= rect2->XPosition)
	{
		dstRect->XPosition = rect2->XPosition;
		v3 = rect1->Width - rect2->XPosition;
		v4 = rect1->XPosition;
	}
	else
	{
		dstRect->XPosition = rect1->XPosition;
		v3 = rect2->Width - rect1->XPosition;
		v4 = rect2->XPosition;
	}
	dstRect->Width = v3 + v4 + 1;
	int v5 = rect1->YPosition;
	if (v5 >= rect2->YPosition)
	{
		dstRect->YPosition = rect2->YPosition;
		v6 = rect1->Height - rect2->YPosition;
		v7 = rect1->YPosition;
	}
	else
	{
		dstRect->YPosition = v5;
		v6 = rect2->Height - rect1->YPosition;
		v7 = rect2->YPosition;
	}
	dstRect->Height = v6 + v7 + 1;
	return dstRect->Width <= rect2->Width + rect1->Width && dstRect->Height <= rect2->Height + rect1->Height;
}

float maths::ray_intersect_circle(ray_type* ray, circle_type* circle)
{
	// O - ray origin
	// D - ray direction
	// C - circle center
	// R - circle radius
	// L, C - O, vector between O and C
	float Lx = circle->Center.X - ray->Origin.X;
	float Ly = circle->Center.Y - ray->Origin.Y;

	// Tca, L dot D, projection of L on D
	float Tca = Ly * ray->Direction.Y + Lx * ray->Direction.X;
	if (Tca < 0.0f) // No intersection if Tca is negative
		return 1000000000.0f;

	// L dot L, distance from ray origin to circle center
	float LMagSq = Ly * Ly + Lx * Lx;

	// If ray origin is inside of the circle
	// T0 = Tca - Sqrt(rad^2 - d^2). d = sqrt(L dot L - Tca dot Tca) 
	if (LMagSq < circle->RadiusSq)
		return Tca - sqrt(circle->RadiusSq - LMagSq + Tca * Tca);

	// Thc^2 = rad^2 - d = rad^2 - L dot L + Tca dot Tca
	float ThcSq = circle->RadiusSq - LMagSq + Tca * Tca;
	if (ThcSq < 0.0f) // No intersection if Thc is negative
		return 1000000000.0f;

	// T0 = Tca - Thc, distance from origin to first intersection
	float T0 = Tca - sqrt(ThcSq);
	if (T0 < 0.0f || T0 > ray->MaxDistance)
		return 1000000000.0f;
	return T0;
}


float maths::normalize_2d(vector_type* vec)
{
	float mag = sqrt(vec->X * vec->X + vec->Y * vec->Y);
	if (mag != 0.0f)
	{
		vec->X = 1.0f / mag * vec->X;
		vec->Y = 1.0f / mag * vec->Y;
	}
	return mag;
}


void maths::line_init(line_type* line, float x0, float y0, float x1, float y1)
{
	float v9;
	bool lineDirection;
	float v11;

	line->Direction.X = x1 - x0;
	line->Direction.Y = y1 - y0;
	normalize_2d(&line->Direction);
	line->PerpendicularL.X = line->Direction.Y;
	line->PerpendicularL.Y = -line->Direction.X;
	line->PreComp1 = -(line->Direction.Y * x0) + line->Direction.X * y0;
	if (line->Direction.X >= 0.000000001f || line->Direction.X <= -0.000000001f)
	{
		v9 = x1;
		lineDirection = x0 >= x1;
		v11 = x0;
	}
	else
	{
		line->Direction.X = 0.0;
		v9 = y1;
		lineDirection = y0 >= y1;
		v11 = y0;
	}
	if (lineDirection)
	{
		line->OriginX = v9;
		line->OriginY = v11;
	}
	else
	{
		line->OriginY = v9;
		line->OriginX = v11;
	}
}

float maths::ray_intersect_line(ray_type* ray, line_type* line)
{
	bool v5;
	bool v6;

	float perpDot = line->PerpendicularL.Y * ray->Direction.Y + ray->Direction.X * line->PerpendicularL.X;
	if (perpDot < 0.0f)
	{
		float result = -((ray->Origin.X * line->PerpendicularL.X + ray->Origin.Y * line->PerpendicularL.Y + line->
				PreComp1)
			/ perpDot);
		if (result >= -ray->MinDistance && result <= ray->MaxDistance)
		{
			line->RayIntersect.X = result * ray->Direction.X + ray->Origin.X;
			float v4 = result * ray->Direction.Y + ray->Origin.Y;
			line->RayIntersect.Y = v4;
			if (line->Direction.X == 0.0f)
			{
				if (v4 >= line->OriginX)
				{
					v5 = v4 < line->OriginY;
					v6 = v4 == line->OriginY;
					if (v5 || v6)
						return result;
					return 1000000000.0;
				}
			}
			else if (line->OriginX <= line->RayIntersect.X)
			{
				float v7 = line->RayIntersect.X;
				v5 = v7 < line->OriginY;
				v6 = v7 == line->OriginY;
				if (v5 || v6)
					return result;
				return 1000000000.0;
			}
		}
	}
	return 1000000000.0;
}

void maths::cross(vector_type* vec1, vector_type* vec2, vector_type* dstVec)
{
	dstVec->X = vec2->Z * vec1->Y - vec2->Y * vec1->Z;
	dstVec->Y = vec2->X * vec1->Z - vec1->X * vec2->Z;
	dstVec->Z = vec1->X * vec2->Y - vec2->X * vec1->Y;
}

float maths::magnitude(vector_type* vec)
{
	float result;
	auto magSq = vec->X * vec->X + vec->Y * vec->Y + vec->Z * vec->Z;
	if (magSq == 0.0f)
		result = 0.0;
	else
		result = sqrt(magSq);
	return result;
}

void maths::vector_add(vector_type* vec1Dst, vector_type* vec2)
{
	vec1Dst->X += vec2->X;
	vec1Dst->Y += vec2->Y;
}

float maths::basic_collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float elasticity, float smoothness,
                             float threshold, float boost)
{
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	float proj = -(direction->Y * ball->Acceleration.Y + direction->X * ball->Acceleration.X);
	if (proj < 0)
	{
		proj = -proj;
	}
	else
	{
		float dx1 = proj * direction->X;
		float dy1 = proj * direction->Y;
		ball->Acceleration.X = (dx1 + ball->Acceleration.X) * smoothness + dx1 * elasticity;
		ball->Acceleration.Y = (dy1 + ball->Acceleration.Y) * smoothness + dy1 * elasticity;
		normalize_2d(&ball->Acceleration);
	}
	float projSpeed = proj * ball->Speed;
	float newSpeed = ball->Speed - (1.0f - elasticity) * projSpeed;
	ball->Speed = newSpeed;
	if (projSpeed >= threshold)
	{
		ball->Acceleration.X = newSpeed * ball->Acceleration.X + direction->X * boost;
		ball->Acceleration.Y = newSpeed * ball->Acceleration.Y + direction->Y * boost;
		ball->Speed = normalize_2d(&ball->Acceleration);
	}
	return projSpeed;
}

float maths::Distance_Squared(vector_type& vec1, vector_type& vec2)
{
	return (vec1.Y - vec2.Y) * (vec1.Y - vec2.Y) + (vec1.X - vec2.X) * (vec1.X - vec2.X);
}

float maths::DotProduct(vector_type* vec1, vector_type* vec2)
{
	return vec1->Y * vec2->Y + vec1->X * vec2->X;
}

void maths::vswap(vector_type* vec1, vector_type* vec2)
{
	vector_type tmp = *vec1;
	*vec1 = *vec2;
	*vec2 = tmp;
}

float maths::Distance(vector_type* vec1, vector_type* vec2)
{
	auto dx = vec1->X - vec2->X;
	auto dy = vec1->Y - vec2->Y;
	return sqrt(dy * dy + dx * dx);
}

void maths::SinCos(float angle, float* sinOut, float* cosOut)
{
	*sinOut = sin(angle);
	*cosOut = cos(angle);
}

void maths::RotatePt(vector_type* point, float sin, float cos, vector_type* origin)
{
	auto dirX = point->X - origin->X;
	auto dirY = point->Y - origin->Y;
	point->X = dirX * cos - dirY * sin + origin->X;
	point->Y = dirX * sin + dirY * cos + origin->Y;
}

float maths::distance_to_flipper(ray_type* ray1, ray_type* ray2)
{
	auto distance = 1000000000.0f;
	auto distanceType = -1;
	auto newDistance = ray_intersect_line(ray1, &TFlipperEdge::lineA);
	if (newDistance < 1000000000.0f)
	{
		distance = newDistance;
		distanceType = 0;
	}
	newDistance = ray_intersect_circle(ray1, &TFlipperEdge::circlebase);
	if (newDistance < distance)
	{
		distance = newDistance;
		distanceType = 2;
	}
	newDistance = ray_intersect_circle(ray1, &TFlipperEdge::circleT1);
	if (newDistance < distance)
	{
		distance = newDistance;
		distanceType = 3;
	}
	newDistance = ray_intersect_line(ray1, &TFlipperEdge::lineB);
	if (newDistance < distance)
	{
		distance = newDistance;
		distanceType = 1;
	}
	if (!ray2 || distance >= 1000000000.0f)
		return distance;

	if (distanceType != -1)
	{
		vector_type* nextOrigin;
		if (distanceType)
		{
			if (distanceType != 1)
			{
				float dirY;
				ray2->Origin.X = distance * ray1->Direction.X + ray1->Origin.X;
				ray2->Origin.Y = distance * ray1->Direction.Y + ray1->Origin.Y;
				if (distanceType == 2)
				{
					ray2->Direction.X = ray2->Origin.X - TFlipperEdge::circlebase.Center.X;
					dirY = ray2->Origin.Y - TFlipperEdge::circlebase.Center.Y;
				}
				else
				{
					ray2->Direction.X = ray2->Origin.X - TFlipperEdge::circleT1.Center.X;
					dirY = ray2->Origin.Y - TFlipperEdge::circleT1.Center.Y;
				}
				ray2->Direction.Y = dirY;
				normalize_2d(&ray2->Direction);
				return distance;
			}
			ray2->Direction = TFlipperEdge::lineB.PerpendicularL;
			nextOrigin = &TFlipperEdge::lineB.RayIntersect;
		}
		else
		{
			ray2->Direction = TFlipperEdge::lineA.PerpendicularL;
			nextOrigin = &TFlipperEdge::lineA.RayIntersect;
		}
		ray2->Origin = *nextOrigin;
		return distance;
	}
	return 1000000000.0;
}

void maths::RotateVector(vector_type* vec, float angle)
{
	float s = sin(angle), c = cos(angle);
	vec->X = c * vec->X - s * vec->Y;
	vec->Y = s * vec->X + c * vec->Y;
	/* Error in the original, should be:
	 * tmp = c * vec->X - s * vec->Y;
	 * vec->Y = s * vec->X + c * vec->Y;
	 * vec->X = tmp
	 */
}

void maths::find_closest_edge(ramp_plane_type* plane, int planeCount, wall_point_type* wall, vector_type** lineEnd,
                              vector_type** lineStart)
{
	vector_type wallEnd{}, wallStart{};

	wallStart.X = wall->X0;
	wallStart.Y = wall->Y0;
	wallEnd.Y = wall->Y1;
	wallEnd.X = wall->X1;

	float maxDistance = 1000000000.0f;
	ramp_plane_type* planePtr = plane;
	for (auto index = 0; index < planeCount; index++)
	{
		auto vec1 = reinterpret_cast<vector_type*>(&planePtr->V1),
		     vec2 = reinterpret_cast<vector_type*>(&planePtr->V2),
		     vec3 = reinterpret_cast<vector_type*>(&planePtr->V3);
		auto distance = Distance(&wallStart, vec1) + Distance(&wallEnd, vec2);
		if (distance < maxDistance)
		{
			maxDistance = distance;
			*lineEnd = vec1;
			*lineStart = vec2;
		}

		distance = Distance(&wallStart, vec2) + Distance(&wallEnd, vec3);
		if (distance < maxDistance)
		{
			maxDistance = distance;
			*lineEnd = vec2;
			*lineStart = vec3;
		}

		distance = Distance(&wallStart, vec3) + Distance(&wallEnd, vec1);
		if (distance < maxDistance)
		{
			maxDistance = distance;
			*lineEnd = vec3;
			*lineStart = vec1;
		}
		++planePtr;
	}
}
