#pragma once
#include "TCollisionComponent.h"

class TPopupTarget :
	public TCollisionComponent2
{
public:
	TPopupTarget(TPinballTable* table, int groupIndex);
	int Message2(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;	

	static void TimerExpired(int timerId, void* caller);

	int Timer;
	float TimerTime;
	int PlayerMessagefieldBackup[4]{};
};
