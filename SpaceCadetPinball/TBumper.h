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
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
		TEdgeSegment* edge) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;
	void Fire();

	static void TimerExpired(int timerId, void* caller);
	
	int BmpIndex;
	int Timer;
	float TimerTime;
	float OriginalThreshold;
	int SoundIndex4;
	int SoundIndex3;
	int Scores[4]{};
	TBumper_player_backup PlayerData[4]{};
};
