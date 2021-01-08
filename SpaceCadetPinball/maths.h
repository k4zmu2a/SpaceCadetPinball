#pragma once

class TBall;

struct vector_type
{
	float X;
	float Y;
	float Z;
};


struct __declspec(align(4)) rectangle_type
{
	int XPosition;
	int YPosition;
	int Width;
	int Height;
};

struct circle_type
{
	vector_type Center;
	float RadiusSq;
};

struct __declspec(align(4)) ray_type
{
	vector_type Origin;
	vector_type Direction;
	float MaxDistance;
	float MinDistance;
	float TimeNow;
	float TimeDelta;
	int FieldFlag;
};

struct __declspec(align(4)) line_type
{
	vector_type PerpendicularL;
	vector_type Direction;
	float PreComp1;
	float OriginX;
	float OriginY;
	vector_type RayIntersect;
};


class maths
{
public:
	static void enclosing_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect);
	static int rectangle_clip(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect);
	static int overlapping_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect);
	static float ray_intersect_circle(ray_type* ray, circle_type* circle);
	static float normalize_2d(vector_type* vec);
	static void line_init(line_type* line, float x0, float y0, float x1, float y1);
	static float ray_intersect_line(ray_type* ray, line_type* line);
	static void cross(vector_type* vec1, vector_type* vec2, vector_type* dstVec);
	static float magnitude(vector_type* vec);
	static void vector_add(vector_type* vec1Dst, vector_type* vec2);
	static float basic_collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float a4, float a5,
	                             float maxSpeed, float multiplier);
	static float Distance_Squared(vector_type vec1, vector_type vec2);
};
