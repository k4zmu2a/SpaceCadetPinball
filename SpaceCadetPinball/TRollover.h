#pragma once
#include "TCollisionComponent.h"

class TRollover :
	public TCollisionComponent
{
protected:
	TRollover(TPinballTable* table, int groupIndex, bool createWall);
public:
	TRollover(TPinballTable* table, int groupIndex);
	~TRollover() override = default;
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;
	void build_walls(int groupIndex);

	static void TimerExpired(int timerId, void* caller);

	char RolloverFlag{};
};
