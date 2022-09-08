#pragma once
#include "maths.h"
#include "TEdgeSegment.h"
#include "TPinballComponent.h"

class TPinballTable;

class TFlipperEdge : public TEdgeSegment
{
public:
	TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup, TPinballTable* table,
	             vector3* origin, vector3* vecT1, vector3* vecT2, float extendTime, float retractTime, float collMult,
	             float elasticity, float smoothness);
	void port_draw() override;
	float FindCollisionDistance(ray_type* ray) override;
	void EdgeCollision(TBall* ball, float distance) override;
	void place_in_grid() override;
	void set_control_points(float timeNow);
	float flipper_angle(float timeNow);
	int is_ball_inside(float x, float y);
	int SetMotion(MessageCode code, float value);

	MessageCode FlipperFlag{};
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
	float AngleSrc{};
	float AngleDst;
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
	float AngleAdvanceTime;
	float ExtendTime;
	float RetractTime;
	vector2 NextBallPosition{};
	vector2 A1, A2, B1, B2, T1;
	line_type lineA, lineB;
	circle_type circlebase, circleT1;
};
