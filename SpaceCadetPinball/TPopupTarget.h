#pragma once
#include "TCollisionComponent.h"

class TPopupTarget :
	public TCollisionComponent
{
public:
	TPopupTarget(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;	

	static void TimerExpired(int timerId, void* caller);

	int Timer;
	float TimerTime;
	int PlayerMessagefieldBackup[4]{};
};
