#pragma once

enum class MessageCode;
class TSink;
class TLight;
class TSound;
class TPinballTable;
class TPinballComponent;
enum class Msg : int;

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

	component_tag(LPCSTR name) : component_tag_base(name), Component(nullptr)
	{
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
	void (& ControlFunc)(MessageCode, TPinballComponent*);
	const unsigned int ScoreCount;
	const int* Scores;
};

struct component_info
{
	component_tag_base& Tag;
	component_control Control;
};


class control
{
public:
	static TPinballTable* TableG;
	static component_info score_components[88];
	static component_tag_base* simple_components[145];
	static int waiting_deployment_flag;
	static bool table_unlimited_balls, easyMode;
	static Msg RankRcArray[9], MissionRcArray[17];
	static int mission_select_scores[17];
	static std::reference_wrapper<TSink*> WormholeSinkArray[3];
	static std::reference_wrapper<TLight*> WormholeLightArray1[3], WormholeLightArray2[3];

	static void make_links(TPinballTable* table);
	static void ClearLinks();
	static TPinballComponent* make_component_link(component_tag_base& tag);
	static void handler(MessageCode code, TPinballComponent* cmp);
	static void pbctrl_bdoor_controller(char key);
	static void table_add_extra_ball(float count);
	static void table_set_bonus_hold();
	static void table_set_bonus();
	static void table_set_jackpot();
	static void table_set_flag_lights();
	static void table_set_multiball(float time);
	static void table_bump_ball_sink_lock();
	static void table_set_replay(float value);
	static void cheat_bump_rank();
	static int SpecialAddScore(int score);
	static int AddRankProgress(int rank);
	static void AdvanceWormHoleDestination(int flag);

	static void FlipperRebounderControl1(MessageCode code, TPinballComponent* caller);
	static void FlipperRebounderControl2(MessageCode code, TPinballComponent* caller);
	static void RebounderControl(MessageCode code, TPinballComponent* caller);
	static void BumperControl(MessageCode code, TPinballComponent* caller);
	static void LeftKickerControl(MessageCode code, TPinballComponent* caller);
	static void RightKickerControl(MessageCode code, TPinballComponent* caller);
	static void LeftKickerGateControl(MessageCode code, TPinballComponent* caller);
	static void RightKickerGateControl(MessageCode code, TPinballComponent* caller);
	static void DeploymentChuteToEscapeChuteOneWayControl(MessageCode code, TPinballComponent* caller);
	static void DeploymentChuteToTableOneWayControl(MessageCode code, TPinballComponent* caller);
	static void DrainBallBlockerControl(MessageCode code, TPinballComponent* caller);
	static void LaunchRampControl(MessageCode code, TPinballComponent* caller);
	static void LaunchRampHoleControl(MessageCode code, TPinballComponent* caller);
	static void SpaceWarpRolloverControl(MessageCode code, TPinballComponent* caller);
	static void ReentryLanesRolloverControl(MessageCode code, TPinballComponent* caller);
	static void BumperGroupControl(MessageCode code, TPinballComponent* caller);
	static void LaunchLanesRolloverControl(MessageCode code, TPinballComponent* caller);
	static void OutLaneRolloverControl(MessageCode code, TPinballComponent* caller);
	static void ExtraBallLightControl(MessageCode code, TPinballComponent* caller);
	static void ReturnLaneRolloverControl(MessageCode code, TPinballComponent* caller);
	static void BonusLaneRolloverControl(MessageCode code, TPinballComponent* caller);
	static void FuelRollover1Control(MessageCode code, TPinballComponent* caller);
	static void FuelRollover2Control(MessageCode code, TPinballComponent* caller);
	static void FuelRollover3Control(MessageCode code, TPinballComponent* caller);
	static void FuelRollover4Control(MessageCode code, TPinballComponent* caller);
	static void FuelRollover5Control(MessageCode code, TPinballComponent* caller);
	static void FuelRollover6Control(MessageCode code, TPinballComponent* caller);
	static void HyperspaceLightGroupControl(MessageCode code, TPinballComponent* caller);
	static void WormHoleControl(MessageCode code, TPinballComponent* caller);
	static void LeftFlipperControl(MessageCode code, TPinballComponent* caller);
	static void RightFlipperControl(MessageCode code, TPinballComponent* caller);
	static void JackpotLightControl(MessageCode code, TPinballComponent* caller);
	static void BonusLightControl(MessageCode code, TPinballComponent* caller);
	static void BoosterTargetControl(MessageCode code, TPinballComponent* caller);
	static void MedalLightGroupControl(MessageCode code, TPinballComponent* caller);
	static void MultiplierLightGroupControl(MessageCode code, TPinballComponent* caller);
	static void FuelSpotTargetControl(MessageCode code, TPinballComponent* caller);
	static void MissionSpotTargetControl(MessageCode code, TPinballComponent* caller);
	static void LeftHazardSpotTargetControl(MessageCode code, TPinballComponent* caller);
	static void RightHazardSpotTargetControl(MessageCode code, TPinballComponent* caller);
	static void WormHoleDestinationControl(MessageCode code, TPinballComponent* caller);
	static void BlackHoleKickoutControl(MessageCode code, TPinballComponent* caller);
	static void FlagControl(MessageCode code, TPinballComponent* caller);
	static void GravityWellKickoutControl(MessageCode code, TPinballComponent* caller);
	static void SkillShotGate1Control(MessageCode code, TPinballComponent* caller);
	static void SkillShotGate2Control(MessageCode code, TPinballComponent* caller);
	static void SkillShotGate3Control(MessageCode code, TPinballComponent* caller);
	static void SkillShotGate4Control(MessageCode code, TPinballComponent* caller);
	static void SkillShotGate5Control(MessageCode code, TPinballComponent* caller);
	static void SkillShotGate6Control(MessageCode code, TPinballComponent* caller);
	static void ShootAgainLightControl(MessageCode code, TPinballComponent* caller);
	static void EscapeChuteSinkControl(MessageCode code, TPinballComponent* caller);
	static void MissionControl(MessageCode code, TPinballComponent* caller);
	static void HyperspaceKickOutControl(MessageCode code, TPinballComponent* caller);
	static void PlungerControl(MessageCode code, TPinballComponent* caller);
	static void MedalTargetControl(MessageCode code, TPinballComponent* caller);
	static void MultiplierTargetControl(MessageCode code, TPinballComponent* caller);
	static void BallDrainControl(MessageCode code, TPinballComponent* caller);

	static void table_control_handler(MessageCode code);

	static void AlienMenaceController(MessageCode code, TPinballComponent* caller);
	static void AlienMenacePartTwoController(MessageCode code, TPinballComponent* caller);
	static void BlackHoleThreatController(MessageCode code, TPinballComponent* caller);
	static void BugHuntController(MessageCode code, TPinballComponent* caller);
	static void CosmicPlagueController(MessageCode code, TPinballComponent* caller);
	static void CosmicPlaguePartTwoController(MessageCode code, TPinballComponent* caller);
	static void DoomsdayMachineController(MessageCode code, TPinballComponent* caller);
	static void GameoverController(MessageCode code, TPinballComponent* caller);
	static void LaunchTrainingController(MessageCode code, TPinballComponent* caller);
	static void MaelstromController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartEightController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartFiveController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartFourController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartSevenController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartSixController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartThreeController(MessageCode code, TPinballComponent* caller);
	static void MaelstromPartTwoController(MessageCode code, TPinballComponent* caller);
	static void PracticeMissionController(MessageCode code, TPinballComponent* caller);
	static void ReconnaissanceController(MessageCode code, TPinballComponent* caller);
	static void ReentryTrainingController(MessageCode code, TPinballComponent* caller);
	static void RescueMissionController(MessageCode code, TPinballComponent* caller);
	static void SatelliteController(MessageCode code, TPinballComponent* caller);
	static void ScienceMissionController(MessageCode code, TPinballComponent* caller);
	static void SecretMissionGreenController(MessageCode code, TPinballComponent* caller);
	static void SecretMissionRedController(MessageCode code, TPinballComponent* caller);
	static void SecretMissionYellowController(MessageCode code, TPinballComponent* caller);
	static void SelectMissionController(MessageCode code, TPinballComponent* caller);
	static void SpaceRadiationController(MessageCode code, TPinballComponent* caller);
	static void StrayCometController(MessageCode code, TPinballComponent* caller);
	static void TimeWarpController(MessageCode code, TPinballComponent* caller);
	static void TimeWarpPartTwoController(MessageCode code, TPinballComponent* caller);
	static void UnselectMissionController(MessageCode code, TPinballComponent* caller);
	static void WaitingDeploymentController(MessageCode code, TPinballComponent* caller);
private:
	static int extraball_light_flag;
};
