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
	int Message(MessageCode code, float value) override;
	vector2 get_coordinates() override;
	void Disable();

	static void throw_ball(TBall* ball, vector3* direction, float angleMult, float speedMult1,
	                       float speedMult2);

	vector3 Position{};
	vector3 Direction{};
	float Speed;
	float RayMaxDistance;
	float TimeDelta;
	float TimeNow;
	vector2 RampFieldForce{};
	TCollisionComponent* CollisionComp;
	int CollisionMask;
	TEdgeSegment* Collisions[5]{};
	int EdgeCollisionCount;
	vector3 CollisionOffset{};
	int CollisionFlag;
	float Offset;
	float VisualZArray[50]{};
};
