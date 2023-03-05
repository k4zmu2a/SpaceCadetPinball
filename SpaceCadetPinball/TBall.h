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
	void throw_ball(vector3* direction, float angleMult, float speedMult1, float speedMult2);

	vector3 Position{};
	vector3 PrevPosition{};
	vector3 Direction{};
	float Speed;
	float RayMaxDistance;
	float TimeDelta;
	float TimeNow;
	vector2 RampFieldForce{};
	TCollisionComponent* CollisionComp;
	int CollisionMask;
	TEdgeSegment* Collisions[16]{};
	int EdgeCollisionCount;
	bool EdgeCollisionResetFlag{};
	vector3 CollisionOffset{};
	int CollisionFlag;
	float Radius;
	bool HasGroupFlag;
	int StuckCounter = 0;
	int LastActiveTime{};
	float VisualZArray[50]{};
	bool AsEdgeCollisionFlag{};
};
