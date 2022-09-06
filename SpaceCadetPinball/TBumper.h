#pragma once
#include "TCollisionComponent.h"

struct  TBumper_player_backup
{
	int MessageField;
	int BmpIndex;
};

class TBumper :
	public TCollisionComponent
{
public:
	TBumper(TPinballTable* table, int groupIndex);
	~TBumper() override = default;
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;
	void Fire();

	static void TimerExpired(int timerId, void* caller);
	
	int BmpIndex;
	int Timer;
	float TimerTime;
	float OriginalThreshold;
	int SoundIndex4;
	int SoundIndex3;
	TBumper_player_backup PlayerData[4]{};
};
