#pragma once

#include "TPinballComponent.h"

class TBall;
struct scoreStruct;
class TFlipper;
class TPlunger;
class TDrain;
class TDemo;
class TLightGroup;

struct score_struct_super
{
	scoreStruct* ScoreStruct;
	int Score;
	int ScoreE9Part;
	int Unknown2;
	int BallCount;
	int ExtraBalls;
	int BallLockedCounter;
};


class TPinballTable : public TPinballComponent
{
public:
	TPinballTable();
	~TPinballTable() override;
	TPinballComponent* find_component(LPCSTR componentName);
	TPinballComponent* find_component(int groupIndex);
	int AddScore(int score);
	void ChangeBallCount(int count);
	void tilt(float time);
	void port_draw() override;
	int Message(int code, float value) override;

	static void EndGame_timeout(int timerId, void* caller);
	static void LightShow_timeout(int timerId, void* caller);
	static void replay_timer_callback(int timerId, void* caller);
	static void tilt_timeout(int timerId, void* caller);

	TFlipper* FlipperL;
	TFlipper* FlipperR;
	scoreStruct* CurScoreStruct;
	scoreStruct* ScoreBallcount;
	scoreStruct* ScorePlayerNumber1;
	int CheatsUsed{};
	int SoundIndex1{};
	int SoundIndex2{};
	int SoundIndex3{};
	int BallInSink;
	int CurScore{};
	int CurScoreE9{};
	int LightShowTimer;
	int EndGameTimeoutTimer;
	int TiltTimeoutTimer;
	score_struct_super PlayerScores[4]{};
	int PlayerCount;
	int CurrentPlayer;
	TPlunger* Plunger;
	TDrain* Drain;
	TDemo* Demo{};
	int XOffset{};
	int YOffset{};
	int Width{};
	int Height{};
	std::vector<TPinballComponent*> ComponentList;
	std::vector<TBall*> BallList;
	TLightGroup* LightGroup;
	float GravityDirVectMult{};
	float GravityAngleX{};
	float GravityAnglY{};
	float CollisionCompOffset{};
	float PlungerPositionX{};
	float PlungerPositionY{};
	int ScoreMultiplier{};
	int ScoreAdded{};
	int ScoreSpecial1{};
	int ScoreSpecial2{};
	int ScoreSpecial2Flag{};
	int ScoreSpecial3{};
	int ScoreSpecial3Flag{};
	int UnknownP71{};
	int BallCount{};
	int MaxBallCount;
	int ExtraBalls{};
	int UnknownP75{};
	int BallLockedCounter{};
	int MultiballFlag;
	int UnknownP78{};
	int ReplayActiveFlag{};
	int ReplayTimer;
	int UnknownP81{};
	int UnknownP82{};
	int TiltLockFlag;

private:
	static int score_multipliers[5];
};
