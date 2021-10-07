#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TPinballTable;

class TFlipperEdge : public TEdgeSegment
{
public:
	TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup, TPinballTable* table,
	             vector_type* origin, vector_type* vecT1, vector_type* vecT2, float extendTime, float retractTime, float collMult,
	             float elasticity, float smoothness);
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
	float Elasticity;
	float Smoothness;
	vector_type RotOrigin{};
	float CirclebaseRadius;
	float CircleT1Radius;
	float CirclebaseRadiusSq;
	float CircleT1RadiusSq;
	float CirclebaseRadiusMSq;
	float CircleT1RadiusMSq;
	float AngleMax;
	float Angle2{};
	float Angle1;
	int CollisionFlag1;
	int CollisionFlag2{};
	vector_type CollisionLinePerp{};
	vector_type A1Src{};
	vector_type A2Src{};
	vector_type B1Src{};
	vector_type B2Src{};
	float CollisionMult;
	vector_type T1Src{};
	vector_type T2Src{};
	float DistanceDivSq;
	float CollisionTimeAdvance;
	vector_type CollisionDirection{};
	int EdgeCollisionFlag;
	float InputTime;
	float AngleStopTime;
	float AngleMult;
	float ExtendTime;
	float RetractTime;
	vector_type NextBallPosition{};

	static float flipper_sin_angle, flipper_cos_angle;
	static vector_type A1, A2, B1, B2, T1;
	static line_type lineA, lineB;
	static circle_type circlebase, circleT1;
};
