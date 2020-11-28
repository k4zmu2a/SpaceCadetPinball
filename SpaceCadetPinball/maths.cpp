#include "pch.h"
#include "maths.h"


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
	int v3; // esi
	int v4; // edi
	int v6; // esi
	int v7; // edi

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
	if (Tca < 0.0) // No intersection if Tca is negative
		return 1000000000.0f;

	// L dot L, distance from ray origin to circle center
	float LMagSq = Ly * Ly + Lx * Lx;

	// If ray origin is inside of the circle
	// T0 = Tca - Sqrt(rad^2 - d^2). d = sqrt(L dot L - Tca dot Tca) 
	if (LMagSq < circle->RadiusSq)
		return Tca - sqrt(circle->RadiusSq - LMagSq + Tca * Tca);

	// Thc^2 = rad^2 - d = rad^2 - L dot L + Tca dot Tca
	float ThcSq = circle->RadiusSq - LMagSq + Tca * Tca;
	if (ThcSq < 0.0) // No intersection if Thc is negative
		return 1000000000.0f;

	// T0 = Tca - Thc, distance from origin to first intersection
	float T0 = Tca - sqrt(ThcSq);
	if (T0 < 0.0 || T0 > ray->MaxDistance)
		return 1000000000.0f;
	return T0;
}


float maths::normalize_2d(vector_type* vec)
{
	float mag = sqrt(vec->X * vec->X + vec->Y * vec->Y);
	if (0.0 != mag)
	{
		vec->X = 1.0f / mag * vec->X;
		vec->Y = 1.0f / mag * vec->Y;
	}
	return mag;
}


void maths::line_init(line_type* line, float x0, float y0, float x1, float y1)
{
	float v9; // st7
	bool lineDirection; // pf
	float v11; // eax

	line->Direction.X = x1 - x0;
	line->Direction.Y = y1 - y0;
	normalize_2d(&line->Direction);
	line->PerpendicularL.X = line->Direction.Y;
	line->PerpendicularL.Y = -line->Direction.X;
	line->PreComp1 = -(line->Direction.Y * x0) + (line->Direction.X * y0);
	if (line->Direction.X >= 0.000000001 || line->Direction.X <= -0.000000001)
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
	// Similar to https://rootllama.wordpress.com/2014/06/20/ray-line-segment-intersection-test-in-2d/
	float perpDot; // st7
	float result; // st7
	float v4; // st6
	bool v5; // c0
	bool v6; // c3
	float v7; // st6

	perpDot = line->PerpendicularL.Y * ray->Direction.Y + ray->Direction.X * line->PerpendicularL.X;
	if (perpDot < 0.0)
	{
		result = -((ray->Origin.X * line->PerpendicularL.X + ray->Origin.Y * line->PerpendicularL.Y + line->PreComp1)
			/ perpDot);
		if (result >= -ray->MinDistance && result <= ray->MaxDistance)
		{
			line->CompTmp1 = result * ray->Direction.X + ray->Origin.X;
			v4 = result * ray->Direction.Y + ray->Origin.Y;
			line->Unknown10 = v4;
			if (0.0 == line->Direction.X)
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
			else if (line->OriginX <= line->CompTmp1)
			{
				v7 = line->CompTmp1;
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
