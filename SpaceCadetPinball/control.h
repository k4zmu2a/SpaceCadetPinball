#pragma once

class TLight;
class TSound;
class TPinballTable;
class TPinballComponent;

struct component_tag_base
{
	LPCSTR Name;

	component_tag_base(LPCSTR name)
	{
		Name = name;
	}

	virtual ~component_tag_base() = default;
	virtual TPinballComponent* GetComponent() = 0;
	virtual void SetComponent(TPinballComponent* comp) = 0;
};

/* Original did not have this template. It supposedly cast TSound and TTextBox from TPinballComponent in tags*/
template <class T = TPinballComponent>
struct component_tag : component_tag_base
{
	static_assert(std::is_base_of<TPinballComponent, T>::value, "T must inherit from TPinballComponent");
	T* Component;

	component_tag(LPCSTR name, TPinballComponent* component): component_tag_base(name), Component(nullptr)
	{
		component_tag::SetComponent(component);
	}

	TPinballComponent* GetComponent() override
	{
		return static_cast<TPinballComponent*>(Component);
	}

	void SetComponent(TPinballComponent* comp) override
	{
		Component = dynamic_cast<T*>(comp);
		if (comp)
			assertm(Component, "Wrong component type");
	}
};


struct component_control
{
	void (* ControlFunc)(int, TPinballComponent*);
	int ScoreCount;
	int* Scores;
};

struct component_info
{
	component_tag_base* Tag;
	component_control Control;
};


class control
{
public:
	static TPinballTable* TableG;
	static component_info score_components[88];
	static component_tag_base* simple_components[142];
	static int waiting_deployment_flag;
	static bool table_unlimited_balls;
	static int RankRcArray[9], MissionRcArray[17], mission_select_scores[17];
	static component_tag_base *wormhole_tag_array1[3], *wormhole_tag_array2[3], *wormhole_tag_array3[3];

	static void make_links(TPinballTable* table);
	static void ClearLinks();
	static TPinballComponent* make_component_link(component_tag_base* tag);
	static void handler(int code, TPinballComponent* cmp);
	static void pbctrl_bdoor_controller(char key);
	static void table_add_extra_ball(float count);
	static void table_set_bonus_hold();
	static void table_set_bonus();
	static void table_set_jackpot();
	static void table_set_flag_lights();
	static void table_set_multiball();
	static void table_bump_ball_sink_lock();
	static void table_set_replay(float value);
	static void cheat_bump_rank();
	static bool light_on(component_tag<TLight>* tag);
	static int SpecialAddScore(int score);
	static int AddRankProgress(int rank);
	static void AdvanceWormHoleDestination(int flag);

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

	static void AlienMenaceController(int code, TPinballComponent* caller);
	static void AlienMenacePartTwoController(int code, TPinballComponent* caller);
	static void BlackHoleThreatController(int code, TPinballComponent* caller);
	static void BugHuntController(int code, TPinballComponent* caller);
	static void CosmicPlagueController(int code, TPinballComponent* caller);
	static void CosmicPlaguePartTwoController(int code, TPinballComponent* caller);
	static void DoomsdayMachineController(int code, TPinballComponent* caller);
	static void GameoverController(int code, TPinballComponent* caller);
	static void LaunchTrainingController(int code, TPinballComponent* caller);
	static void MaelstromController(int code, TPinballComponent* caller);
	static void MaelstromPartEightController(int code, TPinballComponent* caller);
	static void MaelstromPartFiveController(int code, TPinballComponent* caller);
	static void MaelstromPartFourController(int code, TPinballComponent* caller);
	static void MaelstromPartSevenController(int code, TPinballComponent* caller);
	static void MaelstromPartSixController(int code, TPinballComponent* caller);
	static void MaelstromPartThreeController(int code, TPinballComponent* caller);
	static void MaelstromPartTwoController(int code, TPinballComponent* caller);
	static void PracticeMissionController(int code, TPinballComponent* caller);
	static void ReconnaissanceController(int code, TPinballComponent* caller);
	static void ReentryTrainingController(int code, TPinballComponent* caller);
	static void RescueMissionController(int code, TPinballComponent* caller);
	static void SatelliteController(int code, TPinballComponent* caller);
	static void ScienceMissionController(int code, TPinballComponent* caller);
	static void SecretMissionGreenController(int code, TPinballComponent* caller);
	static void SecretMissionRedController(int code, TPinballComponent* caller);
	static void SecretMissionYellowController(int code, TPinballComponent* caller);
	static void SelectMissionController(int code, TPinballComponent* caller);
	static void SpaceRadiationController(int code, TPinballComponent* caller);
	static void StrayCometController(int code, TPinballComponent* caller);
	static void TimeWarpController(int code, TPinballComponent* caller);
	static void TimeWarpPartTwoController(int code, TPinballComponent* caller);
	static void UnselectMissionController(int code, TPinballComponent* caller);
	static void WaitingDeploymentController(int code, TPinballComponent* caller);
private:
	static int extraball_light_flag;
};
