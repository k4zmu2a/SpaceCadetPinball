#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TPinballTable;

class TFlipperEdge : public TEdgeSegment
{
public:
	TFlipperEdge(TCollisionComponent* collComp, char* someFlag, unsigned int visualFlag, TPinballTable* table,
	             vector_type* origin, vector_type* vecT, vector_type* vec3, float bmpCoef1, float bmpCoef2, float a11,
	             float c4F, float c5F);
	void port_draw() override;
	float FindCollisionDistance(ray_type* ray) override;
	void EdgeCollision(TBall* ball, float coef) override;
	void place_in_grid() override;
	void set_control_points(float timeNow);
	void build_edges_in_motion();
	float flipper_angle(float timeNow);
	int is_ball_inside(float x, float y);
	void SetMotion(int code, float value);

	int FlipperFlag;
	float CollisionC4F;
	float CollisionC5F;
	vector_type RotOrigin;
	float CirclebaseRadius;
	float CircleT1Radius;
	float CirclebaseRadiusSq;
	float CircleT1RadiusSq;
	float CirclebaseRadiusMSq;
	float CircleT1RadiusMSq;
	float AngleMax;
	float Angle2;
	float Angle1;
	int Unknown15;
	int Unknown16;
	vector_type Unknown17V;
	vector_type A1Src;
	vector_type A2Src;
	vector_type B1Src;
	vector_type B2Src;
	float Unknown32F;
	vector_type T1Src;
	vector_type Unknown36V;
	float DistanceDivSq;
	float Unknown40F;
	vector_type CollisionDirection;
	int Unknown44;
	float TimeAngle;
	float Unknown46F;
	float AngleMult;
	float BmpCoef1;
	float BmpCoef2;
	vector_type NextBallPosition;

	static float flipper_sin_angle, flipper_cos_angle;
	static vector_type A1, A2, B1, B2, T1;
	static line_type lineA, lineB;
	static circle_type circlebase, circleT1;
};
