#pragma once
#include "maths.h"
#include "TPinballComponent.h"

class TCollisionComponent;
class TEdgeSegment;

class TBall : public TPinballComponent
{
public :
	TBall(TPinballTable* table);
	void Repaint();
	void not_again(TEdgeSegment* edge);
	bool already_hit(TEdgeSegment* edge);
	int Message(int code, float value) override;

	static void throw_ball(TBall* ball, struct vector_type* acceleration, float angleMult, float speedMult1,
	                       float speedMult2);

	vector_type Position{};
	vector_type Acceleration{};
	float Speed;
	float RayMaxDistance;
	float TimeDelta;
	float TimeNow;
	vector_type InvAcceleration{};
	vector_type RampFieldForce{};	
	TCollisionComponent* CollisionComp;
	int FieldFlag;
	TEdgeSegment* Collisions[5]{};
	int EdgeCollisionCount;
	vector_type CollisionOffset{};
	int CollisionFlag;
	float Offset;
	float VisualZArray[50]{};
};
