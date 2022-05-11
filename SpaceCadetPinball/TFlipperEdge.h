#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TPinballTable;

class TFlipperEdge : public TEdgeSegment
{
public:
	TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup, TPinballTable* table,
	             vector3* origin, vector3* vecT1, vector3* vecT2, float extendTime, float retractTime, float collMult,
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
	vector2 RotOrigin{};
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
	vector2 CollisionLinePerp{};
	vector2 A1Src{};
	vector2 A2Src{};
	vector2 B1Src{};
	vector2 B2Src{};
	float CollisionMult;
	vector2 T1Src{};
	vector2 T2Src{};
	float DistanceDivSq;
	float CollisionTimeAdvance;
	vector2 CollisionDirection{};
	int EdgeCollisionFlag;
	float InputTime;
	float AngleStopTime;
	float AngleMult;
	float ExtendTime;
	float RetractTime;
	vector2 NextBallPosition{};

	static float flipper_sin_angle, flipper_cos_angle;
	static vector2 A1, A2, B1, B2, T1;
	static line_type lineA, lineB;
	static circle_type circlebase, circleT1;
};
