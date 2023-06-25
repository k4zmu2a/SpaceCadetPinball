#pragma once

#include "maths.h"
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
	int JackpotScore;
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
	int Message(MessageCode code, float value) override;
	TBall* AddBall(vector2 position);
	int BallCountInRect(const RectF& rect);
	int BallCountInRect(const vector2& pos, float margin);

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
	int BallInDrainFlag;
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
	std::vector<TFlipper*> FlipperList;
	TLightGroup* LightGroup;
	float GravityDirVectMult{};
	float GravityAngleX{};
	float GravityAnglY{};
	float CollisionCompOffset{};
	vector2 PlungerPosition{};
	int ScoreMultiplier{};
	int ScoreAdded{};
	int ReflexShotScore{};
	int BonusScore{};
	bool BonusScoreFlag{};
	int JackpotScore{};
	bool JackpotScoreFlag{};
	int UnknownP71{};
	int BallCount{};
	int MaxBallCount;
	int ExtraBalls{};
	int MultiballCount{};
	int BallLockedCounter{};
	bool MultiballFlag;
	int UnknownP78{};
	int ReplayActiveFlag{};
	int ReplayTimer;
	int UnknownP81{};
	int UnknownP82{};
	int TiltLockFlag;

private:
	static int score_multipliers[5];
};
