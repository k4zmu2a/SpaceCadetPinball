#pragma once

class TBall;

struct vector_type
{
	float X;
	float Y;
	float Z;
};


struct rectangle_type
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

struct ray_type
{
	vector_type Origin;
	vector_type Direction;
	float MaxDistance;
	float MinDistance;
	float TimeNow;
	float TimeDelta;
	int FieldFlag;
};

struct line_type
{
	vector_type PerpendicularL;
	vector_type Direction;
	float PreComp1;
	float OriginX;
	float OriginY;
	vector_type RayIntersect;
};

struct vector_type2
{
	float X;
	float Y;
};

struct wall_point_type
{
	float X0;
	float Y0;
	float X1;
	float Y1;
};

struct ramp_plane_type
{
	vector_type BallCollisionOffset;
	vector_type2 V1;
	vector_type2 V2;
	vector_type2 V3;
	float GravityAngle1;
	float GravityAngle2;
	vector_type2 FieldForce;
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
	static float basic_collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float elasticity,
	                             float smoothness,
	                             float threshold, float boost);
	static float Distance_Squared(vector_type& vec1, vector_type& vec2);
	static float DotProduct(vector_type* vec1, vector_type* vec2);
	static void vswap(vector_type* vec1, vector_type* vec2);
	static float Distance(vector_type* vec1, vector_type* vec2);
	static void SinCos(float angle, float* sinOut, float* cosOut);
	static void RotatePt(vector_type* point, float sin, float cos, vector_type* origin);
	static float distance_to_flipper(ray_type* ray1, ray_type* ray2);
	static void RotateVector(vector_type* vec, float angle);
	static void find_closest_edge(ramp_plane_type* plane, int planeCount, wall_point_type* wall, vector_type** lineEnd,
	                              vector_type** lineStart);
};
