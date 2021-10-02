#pragma once
#include "TCollisionComponent.h"

class TPopupTarget :
	public TCollisionComponent
{
public:
	TPopupTarget(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
		TEdgeSegment* edge) override;	

	static void TimerExpired(int timerId, void* caller);

	int Timer;
	float TimerTime;
	int Scores[3]{};
	int PlayerMessagefieldBackup[4]{};
};
