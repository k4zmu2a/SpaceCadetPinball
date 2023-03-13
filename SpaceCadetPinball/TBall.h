#pragma once
#include "maths.h"
#include "TCollisionComponent.h"
#include "TEdgeSegment.h"

class TBall : public TCollisionComponent, public TEdgeSegment
{
public :
	TBall(TPinballTable* table, int groupIndex);
	void Repaint();
	void not_again(TEdgeSegment* edge);
	bool already_hit(const TEdgeSegment& edge) const;
	int Message(MessageCode code, float value) override;
	vector2 get_coordinates() override;
	void Disable();
	void throw_ball(vector3* direction, float angleMult, float speedMult1, float speedMult2);
	void place_in_grid(RectF* aabb) override {}
	void EdgeCollision(TBall* ball, float distance) override;
	float FindCollisionDistance(const ray_type& ray) override;

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
	bool CollisionDisabledFlag{};
};
