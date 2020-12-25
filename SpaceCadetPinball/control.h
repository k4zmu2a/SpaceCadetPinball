#pragma once

class TPinballTable;
class TPinballComponent;

struct component_tag
{
	LPCSTR Name;
	TPinballComponent* Component;
};


struct component_control
{
	void (* ControlFunc)(int, TPinballComponent*);
	int ScoreCount;
	int* Scores;
};

struct component_info
{
	component_tag* Tag;
	component_control Control;
};


class control
{
public:
	static TPinballTable* TableG;
	static component_info score_components[88];
	static component_tag* simple_components[142];
	static int table_control_flag;

	static void make_links(TPinballTable* table);
	static TPinballComponent* make_component_link(component_tag* tag);
	static void handler(int code, TPinballComponent* cmp);

	static void FlipperRebounderControl1(int code, TPinballComponent* caller);
	static void FlipperRebounderControl2(int code, TPinballComponent* caller);
	static void RebounderControl(int code, TPinballComponent* caller);
	static void BumperControl(int code, TPinballComponent* caller);
	static void LeftKickerControl(int code, TPinballComponent* caller);
	static void RightKickerControl(int code, TPinballComponent* caller);
	static void LeftKickerGateControl(int code, TPinballComponent* caller);
	static void RightKickerGateControl(int code, TPinballComponent* caller);
	static void DeploymentChuteToEscapeChuteOneWayControl(int code, TPinballComponent* caller);
	static void DeploymentChuteToTableOneWayControl(int code, TPinballComponent* caller);
	static void DrainBallBlockerControl(int code, TPinballComponent* caller);
	static void LaunchRampControl(int code, TPinballComponent* caller);
	static void LaunchRampHoleControl(int code, TPinballComponent* caller);
	static void SpaceWarpRolloverControl(int code, TPinballComponent* caller);
	static void ReentryLanesRolloverControl(int code, TPinballComponent* caller);
	static void BumperGroupControl(int code, TPinballComponent* caller);
	static void LaunchLanesRolloverControl(int code, TPinballComponent* caller);
	static void OutLaneRolloverControl(int code, TPinballComponent* caller);
	static void ExtraBallLightControl(int code, TPinballComponent* caller);
	static void ReturnLaneRolloverControl(int code, TPinballComponent* caller);
	static void BonusLaneRolloverControl(int code, TPinballComponent* caller);
	static void FuelRollover1Control(int code, TPinballComponent* caller);
	static void FuelRollover2Control(int code, TPinballComponent* caller);
	static void FuelRollover3Control(int code, TPinballComponent* caller);
	static void FuelRollover4Control(int code, TPinballComponent* caller);
	static void FuelRollover5Control(int code, TPinballComponent* caller);
	static void FuelRollover6Control(int code, TPinballComponent* caller);
	static void HyperspaceLightGroupControl(int code, TPinballComponent* caller);
	static void WormHoleControl(int code, TPinballComponent* caller);
	static void LeftFlipperControl(int code, TPinballComponent* caller);
	static void RightFlipperControl(int code, TPinballComponent* caller);
	static void JackpotLightControl(int code, TPinballComponent* caller);
	static void BonusLightControl(int code, TPinballComponent* caller);
	static void BoosterTargetControl(int code, TPinballComponent* caller);
	static void MedalLightGroupControl(int code, TPinballComponent* caller);
	static void MultiplierLightGroupControl(int code, TPinballComponent* caller);
	static void FuelSpotTargetControl(int code, TPinballComponent* caller);
	static void MissionSpotTargetControl(int code, TPinballComponent* caller);
	static void LeftHazardSpotTargetControl(int code, TPinballComponent* caller);
	static void RightHazardSpotTargetControl(int code, TPinballComponent* caller);
	static void WormHoleDestinationControl(int code, TPinballComponent* caller);
	static void BlackHoleKickoutControl(int code, TPinballComponent* caller);
	static void FlagControl(int code, TPinballComponent* caller);
	static void GravityWellKickoutControl(int code, TPinballComponent* caller);
	static void SkillShotGate1Control(int code, TPinballComponent* caller);
	static void SkillShotGate2Control(int code, TPinballComponent* caller);
	static void SkillShotGate3Control(int code, TPinballComponent* caller);
	static void SkillShotGate4Control(int code, TPinballComponent* caller);
	static void SkillShotGate5Control(int code, TPinballComponent* caller);
	static void SkillShotGate6Control(int code, TPinballComponent* caller);
	static void ShootAgainLightControl(int code, TPinballComponent* caller);
	static void EscapeChuteSinkControl(int code, TPinballComponent* caller);
	static void MissionControl(int code, TPinballComponent* caller);
	static void HyperspaceKickOutControl(int code, TPinballComponent* caller);
	static void PlungerControl(int code, TPinballComponent* caller);
	static void MedalTargetControl(int code, TPinballComponent* caller);
	static void MultiplierTargetControl(int code, TPinballComponent* caller);
	static void BallDrainControl(int code, TPinballComponent* caller);

	static void table_control_handler(int code);	
};
