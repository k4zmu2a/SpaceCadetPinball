#pragma once
#include "maths.h"
#include "TEdgeSegment.h"
#include "TPinballComponent.h"

class TPinballTable;

class TFlipperEdge : public TEdgeSegment
{
public:
	TFlipperEdge(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup, TPinballTable* table,
	             vector3* origin, vector3* vecT1, vector3* vecT2, float extendSpeed, float retractSpeed, float collMult,
	             float elasticity, float smoothness);
	void port_draw() override;
	float FindCollisionDistance(ray_type* ray) override;
	void EdgeCollision(TBall* ball, float distance) override;
	void place_in_grid(RectF* aabb) override;
	void set_control_points(float angle);
	float flipper_angle_delta(float timeDelta);
	int SetMotion(MessageCode code);

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
	float AngleRemainder{};
	float AngleDst;
	float CurrentAngle{};
	vector2 CollisionLinePerp{};
	vector2 A1Src{};
	vector2 A2Src{};
	vector2 B1Src{};
	vector2 B2Src{};
	float CollisionMult;
	vector2 T1Src{};
	vector2 T2Src{};
	float DistanceDiv, DistanceDivSq;
	vector2 CollisionDirection{};
	float ExtendSpeed;
	float RetractSpeed;
	float MoveSpeed;
	vector2 NextBallPosition{};
	vector2 A1, A2, B1, B2, T1;
	line_type LineA, LineB;
	circle_type circlebase, circleT1;
	float InvT1Radius;
	float YMin, YMax, XMin, XMax;
	bool ControlPointDirtyFlag{};
};
