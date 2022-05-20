#pragma once

class TBall;

struct vector2
{
	float X;
	float Y;

	bool operator==(const vector2& vec)
	{
		return X == vec.X && Y == vec.Y;
	}
	bool operator!=(const vector2& vec)
	{
		return X != vec.X || Y != vec.Y;
	}
};

struct vector3 :vector2
{	
	vector3() = default;
	vector3(float x, float y) : vector3{ x, y, 0 } {}
	vector3(float x, float y, float z) : vector2{ x, y }, Z(z) {}
	float Z;
};

struct vector2i
{
	int X;
	int Y;
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
	vector2 Center;
	float RadiusSq;
};

struct ray_type
{
	vector2 Origin;
	vector2 Direction;
	float MaxDistance;
	float MinDistance;
	float TimeNow;
	float TimeDelta;
	int CollisionMask;
};

struct line_type
{
	vector2 PerpendicularC;
	vector2 Direction;
	vector2 Origin;
	vector2 End;
	float MinCoord;
	float MaxCoord;
	vector2 RayIntersect;
};

struct wall_point_type
{
	vector2 Pt0;
	vector2 Pt1;
};

struct ramp_plane_type
{
	vector3 BallCollisionOffset;
	vector2 V1;
	vector2 V2;
	vector2 V3;
	float GravityAngle1;
	float GravityAngle2;
	vector2 FieldForce;
};

enum class FlipperIntersect
{
	none = -1,
	lineA = 0,
	lineB = 1,
	circlebase = 2,
	circleT1 = 3
};


class maths
{
public:
	static void enclosing_box(const rectangle_type& rect1, const rectangle_type& rect2, rectangle_type& dstRect);
	static bool rectangle_clip(const rectangle_type& rect1, const rectangle_type& rect2, rectangle_type* dstRect);
	static float ray_intersect_circle(const ray_type& ray, const circle_type& circle);
	static float normalize_2d(vector2& vec);
	static void line_init(line_type& line, float x0, float y0, float x1, float y1);
	static float ray_intersect_line(const ray_type& ray, line_type& line);
	static void cross(const vector3& vec1, const vector3& vec2, vector3& dstVec);
	static float cross(const vector2& vec1, const vector2& vec2);
	static float magnitude(const vector3& vec);
	static void vector_add(vector2& vec1Dst, const vector2& vec2);
	static vector2 vector_sub(const vector2& vec1, const vector2& vec2);
	static vector2 vector_mul(const vector2& vec1, float val);
	static float basic_collision(TBall* ball, vector2* nextPosition, vector2* direction, float elasticity,
	                             float smoothness,
	                             float threshold, float boost);
	static float Distance_Squared(const vector2& vec1, const vector2& vec2);
	static float DotProduct(const vector2& vec1, const vector2& vec2);
	static float Distance(const vector2& vec1, const vector2& vec2);
	static void SinCos(float angle, float& sinOut, float& cosOut);
	static void RotatePt(vector2& point, float sin, float cos, const vector2& origin);
	static float distance_to_flipper(const ray_type& ray1, ray_type& ray2);
	static void RotateVector(vector2& vec, float angle);
	static void find_closest_edge(ramp_plane_type* plane, int planeCount, wall_point_type* wall, vector2& lineEnd,
	                              vector2& lineStart);
};
