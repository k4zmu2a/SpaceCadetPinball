#pragma once
#include "maths.h"
#include "TCollisionComponent.h"

class TSink :
	public TCollisionComponent
{
public:
	TSink(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	               TEdgeSegment* edge) override;

	static void TimerExpired(int timerId, void* caller);

	float TimerTime;
	vector2 BallPosition{};
	vector3 BallThrowDirection{};
	float ThrowAngleMult;
	float ThrowSpeedMult1;
	float ThrowSpeedMult2;
	int SoundIndex4;
	int SoundIndex3;
	int PlayerMessagefieldBackup[4]{};
};
