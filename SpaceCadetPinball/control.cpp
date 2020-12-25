#include "pch.h"
#include "control.h"

#include "objlist_class.h"
#include "TPinballTable.h"

int control_bump_scores1[] = {500, 1000, 1500, 2000};
int control_roll_scores1[] = {2000};
int control_bump_scores2[] = {1500, 2500, 3500, 4500};
int control_roll_scores2[] = {500};
int control_rebo_score1[] = {500};
int control_oneway4_score1[6] = {15000, 30000, 75000, 30000, 15000, 7500};
int control_ramp_score1[1] = {5000};
int control_roll_score1[1] = {20000};
int control_roll_score2[2] = {5000, 25000};
int control_roll_score3[1] = {10000};
int control_roll_score4[1] = {500};
int control_flag_score1[2] = {500, 2500};
int control_kickout_score1[5] = {10000, 0, 20000, 50000, 150000};
int control_sink_score1[3] = {2500, 5000, 7500};
int control_target_score1[2] = {500, 5000};
int control_target_score2[3] = {1500, 10000, 50000};
int control_target_score3[2] = {500, 1500};
int control_target_score4[1] = {750};
int control_target_score5[1] = {1000};
int control_target_score6[1] = {750};
int control_target_score7[1] = {750};
int control_roll_score5[1] = {10000};
int control_kickout_score2[1] = {20000};
int control_kickout_score3[1] = {50000};


component_tag control_attack_bump_tag = {"attack_bumpers", nullptr};
component_tag control_block1_tag = {"v_bloc1", nullptr};
component_tag control_bmpr_inc_lights_tag = {"bmpr_inc_lights", nullptr};
component_tag control_bpr_solotgt_lights_tag = {"bpr_solotgt_lights", nullptr};
component_tag control_bsink_arrow_lights_tag = {"bsink_arrow_lights", nullptr};
component_tag control_bumber_target_lights_tag = {"bumper_target_lights", nullptr};
component_tag control_bump1_tag = {"a_bump1", nullptr};
component_tag control_bump2_tag = {"a_bump2", nullptr};
component_tag control_bump3_tag = {"a_bump3", nullptr};
component_tag control_bump4_tag = {"a_bump4", nullptr};
component_tag control_bump5_tag = {"a_bump5", nullptr};
component_tag control_bump6_tag = {"a_bump6", nullptr};
component_tag control_bump7_tag = {"a_bump7", nullptr};
component_tag control_drain_tag = {"drain", nullptr};
component_tag control_flag1_tag = {"a_flag1", nullptr};
component_tag control_flag2_tag = {"a_flag2", nullptr};
component_tag control_flip1_tag = {"a_flip1", nullptr};
component_tag control_flip2_tag = {"a_flip2", nullptr};
component_tag control_fuel_bargraph_tag = {"fuel_bargraph", nullptr};
component_tag control_gate1_tag = {"v_gate1", nullptr};
component_tag control_gate2_tag = {"v_gate2", nullptr};
component_tag control_goal_lights_tag = {"goal_lights", nullptr};
component_tag control_hyper_lights_tag = {"hyperspace_lights", nullptr};
component_tag control_info_text_box_tag = {"info_text_box", nullptr};
component_tag control_kicker1_tag = {"a_kick1", nullptr};
component_tag control_kicker2_tag = {"a_kick2", nullptr};
component_tag control_kickout1_tag = {"a_kout1", nullptr};
component_tag control_kickout2_tag = {"a_kout2", nullptr};
component_tag control_kickout3_tag = {"a_kout3", nullptr};
component_tag control_l_trek_lights_tag = {"l_trek_lights", nullptr};
component_tag control_launch_bump_tag = {"launch_bumpers", nullptr};
component_tag control_lchute_tgt_lights_tag = {"lchute_tgt_lights", nullptr};
component_tag control_lite1_tag = {"lite1", nullptr};
component_tag control_lite2_tag = {"lite2", nullptr};
component_tag control_lite3_tag = {"lite3", nullptr};
component_tag control_lite4_tag = {"lite4", nullptr};
component_tag control_lite5_tag = {"lite5", nullptr};
component_tag control_lite6_tag = {"lite6", nullptr};
component_tag control_lite7_tag = {"lite7", nullptr};
component_tag control_lite8_tag = {"lite8", nullptr};
component_tag control_lite9_tag = {"lite9", nullptr};
component_tag control_lite10_tag = {"lite10", nullptr};
component_tag control_lite11_tag = {"lite11", nullptr};
component_tag control_lite12_tag = {"lite12", nullptr};
component_tag control_lite13_tag = {"lite13", nullptr};
component_tag control_lite16_tag = {"lite16", nullptr};
component_tag control_lite17_tag = {"lite17", nullptr};
component_tag control_lite18_tag = {"lite18", nullptr};
component_tag control_lite19_tag = {"lite19", nullptr};
component_tag control_lite20_tag = {"lite20", nullptr};
component_tag control_lite21_tag = {"lite21", nullptr};
component_tag control_lite22_tag = {"lite22", nullptr};
component_tag control_lite23_tag = {"lite23", nullptr};
component_tag control_lite24_tag = {"lite24", nullptr};
component_tag control_lite25_tag = {"lite25", nullptr};
component_tag control_lite26_tag = {"lite26", nullptr};
component_tag control_lite27_tag = {"lite27", nullptr};
component_tag control_lite28_tag = {"lite28", nullptr};
component_tag control_lite29_tag = {"lite29", nullptr};
component_tag control_lite30_tag = {"lite30", nullptr};
component_tag control_lite54_tag = {"lite54", nullptr};
component_tag control_lite55_tag = {"lite55", nullptr};
component_tag control_lite56_tag = {"lite56", nullptr};
component_tag control_lite58_tag = {"lite58", nullptr};
component_tag control_lite59_tag = {"lite59", nullptr};
component_tag control_lite60_tag = {"lite60", nullptr};
component_tag control_lite61_tag = {"lite61", nullptr};
component_tag control_lite62_tag = {"lite62", nullptr};
component_tag control_lite67_tag = {"lite67", nullptr};
component_tag control_lite68_tag = {"lite68", nullptr};
component_tag control_lite69_tag = {"lite69", nullptr};
component_tag control_lite70_tag = {"lite70", nullptr};
component_tag control_lite71_tag = {"lite71", nullptr};
component_tag control_lite72_tag = {"lite72", nullptr};
component_tag control_lite77_tag = {"lite77", nullptr};
component_tag control_lite84_tag = {"lite84", nullptr};
component_tag control_lite85_tag = {"lite85", nullptr};
component_tag control_lite101_tag = {"lite101", nullptr};
component_tag control_lite102_tag = {"lite102", nullptr};
component_tag control_lite103_tag = {"lite103", nullptr};
component_tag control_lite104_tag = {"lite104", nullptr};
component_tag control_lite105_tag = {"lite105", nullptr};
component_tag control_lite106_tag = {"lite106", nullptr};
component_tag control_lite107_tag = {"lite107", nullptr};
component_tag control_lite108_tag = {"lite108", nullptr};
component_tag control_lite109_tag = {"lite109", nullptr};
component_tag control_lite110_tag = {"lite110", nullptr};
component_tag control_lite130_tag = {"lite130", nullptr};
component_tag control_lite131_tag = {"lite131", nullptr};
component_tag control_lite132_tag = {"lite132", nullptr};
component_tag control_lite133_tag = {"lite133", nullptr};
component_tag control_lite169_tag = {"lite169", nullptr};
component_tag control_lite170_tag = {"lite170", nullptr};
component_tag control_lite171_tag = {"lite171", nullptr};
component_tag control_lite195_tag = {"lite195", nullptr};
component_tag control_lite196_tag = {"lite196", nullptr};
component_tag control_lite198_tag = {"lite198", nullptr};
component_tag control_lite199_tag = {"lite199", nullptr};
component_tag control_lite200_tag = {"lite200", nullptr};
component_tag control_lite300_tag = {"lite300", nullptr};
component_tag control_lite301_tag = {"lite301", nullptr};
component_tag control_lite302_tag = {"lite302", nullptr};
component_tag control_lite303_tag = {"lite303", nullptr};
component_tag control_lite304_tag = {"lite304", nullptr};
component_tag control_lite305_tag = {"lite305", nullptr};
component_tag control_lite306_tag = {"lite306", nullptr};
component_tag control_lite307_tag = {"lite307", nullptr};
component_tag control_lite308_tag = {"lite308", nullptr};
component_tag control_lite309_tag = {"lite309", nullptr};
component_tag control_lite310_tag = {"lite310", nullptr};
component_tag control_lite311_tag = {"lite311", nullptr};
component_tag control_lite312_tag = {"lite312", nullptr};
component_tag control_lite313_tag = {"lite313", nullptr};
component_tag control_lite314_tag = {"lite314", nullptr};
component_tag control_lite315_tag = {"lite315", nullptr};
component_tag control_lite316_tag = {"lite316", nullptr};
component_tag control_lite317_tag = {"lite317", nullptr};
component_tag control_lite318_tag = {"lite318", nullptr};
component_tag control_lite319_tag = {"lite319", nullptr};
component_tag control_lite320_tag = {"lite320", nullptr};
component_tag control_lite321_tag = {"lite321", nullptr};
component_tag control_lite322_tag = {"lite322", nullptr};
component_tag control_literoll179_tag = {"literoll179", nullptr};
component_tag control_literoll180_tag = {"literoll180", nullptr};
component_tag control_literoll181_tag = {"literoll181", nullptr};
component_tag control_literoll182_tag = {"literoll182", nullptr};
component_tag control_literoll183_tag = {"literoll183", nullptr};
component_tag control_literoll184_tag = {"literoll184", nullptr};
component_tag control_middle_circle_tag = {"middle_circle", nullptr};
component_tag control_mission_text_box_tag = {"mission_text_box", nullptr};
component_tag control_oneway1_tag = {"s_onewy1", nullptr};
component_tag control_oneway4_tag = {"s_onewy4", nullptr};
component_tag control_oneway10_tag = {"s_onewy10", nullptr};
component_tag control_outer_circle_tag = {"outer_circle", nullptr};
component_tag control_plunger_tag = {"plunger", nullptr};
component_tag control_r_trek_lights_tag = {"r_trek_lights", nullptr};
component_tag control_ramp_bmpr_inc_lights_tag = {"ramp_bmpr_inc_lights", nullptr};
component_tag control_ramp_hole_tag = {"ramp_hole", nullptr};
component_tag control_ramp_tag = {"ramp", nullptr};
component_tag control_ramp_tgt_lights_tag = {"ramp_tgt_lights", nullptr};
component_tag control_rebo1_tag = {"v_rebo1", nullptr};
component_tag control_rebo2_tag = {"v_rebo2", nullptr};
component_tag control_rebo3_tag = {"v_rebo3", nullptr};
component_tag control_rebo4_tag = {"v_rebo4", nullptr};
component_tag control_roll1_tag = {"a_roll1", nullptr};
component_tag control_roll2_tag = {"a_roll2", nullptr};
component_tag control_roll3_tag = {"a_roll3", nullptr};
component_tag control_roll4_tag = {"a_roll4", nullptr};
component_tag control_roll5_tag = {"a_roll5", nullptr};
component_tag control_roll6_tag = {"a_roll6", nullptr};
component_tag control_roll7_tag = {"a_roll7", nullptr};
component_tag control_roll8_tag = {"a_roll8", nullptr};
component_tag control_roll9_tag = {"a_roll9", nullptr};
component_tag control_roll110_tag = {"a_roll110", nullptr};
component_tag control_roll111_tag = {"a_roll111", nullptr};
component_tag control_roll112_tag = {"a_roll112", nullptr};
component_tag control_roll179_tag = {"a_roll179", nullptr};
component_tag control_roll180_tag = {"a_roll180", nullptr};
component_tag control_roll181_tag = {"a_roll181", nullptr};
component_tag control_roll182_tag = {"a_roll182", nullptr};
component_tag control_roll183_tag = {"a_roll183", nullptr};
component_tag control_roll184_tag = {"a_roll184", nullptr};
component_tag control_sink1_tag = {"v_sink1", nullptr};
component_tag control_sink2_tag = {"v_sink2", nullptr};
component_tag control_sink3_tag = {"v_sink3", nullptr};
component_tag control_sink7_tag = {"v_sink7", nullptr};
component_tag control_skill_shot_lights_tag = {"skill_shot_lights", nullptr};
component_tag control_soundwave3_tag = {"soundwave3", nullptr};
component_tag control_soundwave7_tag = {"soundwave7", nullptr};
component_tag control_soundwave8_tag = {"soundwave8", nullptr};
component_tag control_soundwave9_tag = {"soundwave9", nullptr};
component_tag control_soundwave10_tag = {"soundwave10", nullptr};
component_tag control_soundwave14_1_tag = {"soundwave14", nullptr};
component_tag control_soundwave14_2_tag = {"soundwave14", nullptr};
component_tag control_soundwave21_tag = {"soundwave21", nullptr};
component_tag control_soundwave23_tag = {"soundwave23", nullptr};
component_tag control_soundwave24_tag = {"soundwave24", nullptr};
component_tag control_soundwave25_tag = {"soundwave25", nullptr};
component_tag control_soundwave26_tag = {"soundwave26", nullptr};
component_tag control_soundwave27_tag = {"soundwave27", nullptr};
component_tag control_soundwave28_tag = {"soundwave28", nullptr};
component_tag control_soundwave30_tag = {"soundwave30", nullptr};
component_tag control_soundwave35_1_tag = {"soundwave35", nullptr};
component_tag control_soundwave35_2_tag = {"soundwave35", nullptr};
component_tag control_soundwave36_1_tag = {"soundwave36", nullptr};
component_tag control_soundwave36_2_tag = {"soundwave36", nullptr};
component_tag control_soundwave38_tag = {"soundwave38", nullptr};
component_tag control_soundwave39_tag = {"soundwave39", nullptr};
component_tag control_soundwave40_tag = {"soundwave40", nullptr};
component_tag control_soundwave41_tag = {"soundwave41", nullptr};
component_tag control_soundwave44_tag = {"soundwave44", nullptr};
component_tag control_soundwave45_tag = {"soundwave45", nullptr};
component_tag control_soundwave46_tag = {"soundwave46", nullptr};
component_tag control_soundwave47_tag = {"soundwave47", nullptr};
component_tag control_soundwave48_tag = {"soundwave48", nullptr};
component_tag control_soundwave49D_tag = {"soundwave49D", nullptr};
component_tag control_soundwave50_1_tag = {"soundwave50", nullptr};
component_tag control_soundwave50_2_tag = {"soundwave50", nullptr};
component_tag control_soundwave52_tag = {"soundwave52", nullptr};
component_tag control_soundwave59_tag = {"soundwave59", nullptr};
component_tag control_target1_tag = {"a_targ1", nullptr};
component_tag control_target2_tag = {"a_targ2", nullptr};
component_tag control_target3_tag = {"a_targ3", nullptr};
component_tag control_target4_tag = {"a_targ4", nullptr};
component_tag control_target5_tag = {"a_targ5", nullptr};
component_tag control_target6_tag = {"a_targ6", nullptr};
component_tag control_target7_tag = {"a_targ7", nullptr};
component_tag control_target8_tag = {"a_targ8", nullptr};
component_tag control_target9_tag = {"a_targ9", nullptr};
component_tag control_target10_tag = {"a_targ10", nullptr};
component_tag control_target11_tag = {"a_targ11", nullptr};
component_tag control_target12_tag = {"a_targ12", nullptr};
component_tag control_target13_tag = {"a_targ13", nullptr};
component_tag control_target14_tag = {"a_targ14", nullptr};
component_tag control_target15_tag = {"a_targ15", nullptr};
component_tag control_target16_tag = {"a_targ16", nullptr};
component_tag control_target17_tag = {"a_targ17", nullptr};
component_tag control_target18_tag = {"a_targ18", nullptr};
component_tag control_target19_tag = {"a_targ19", nullptr};
component_tag control_target20_tag = {"a_targ20", nullptr};
component_tag control_target21_tag = {"a_targ21", nullptr};
component_tag control_target22_tag = {"a_targ22", nullptr};
component_tag control_top_circle_tgt_lights_tag = {"top_circle_tgt_lights", nullptr};
component_tag control_top_target_lights_tag = {"top_target_lights", nullptr};
component_tag control_trip1_tag = {"s_trip1", nullptr};
component_tag control_trip2_tag = {"s_trip2", nullptr};
component_tag control_trip3_tag = {"s_trip3", nullptr};
component_tag control_trip4_tag = {"s_trip4", nullptr};
component_tag control_trip5_tag = {"s_trip5", nullptr};
component_tag control_worm_hole_lights_tag = {"worm_hole_lights", nullptr};


TPinballTable* control::TableG;
component_info control::score_components[88]
{
	component_info{&control_bump1_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_bump2_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_bump3_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_bump4_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_roll3_tag, {ReentryLanesRolloverControl, 1, control_roll_scores1}},
	component_info{&control_roll2_tag, {ReentryLanesRolloverControl, 1, control_roll_scores1}},
	component_info{&control_roll1_tag, {ReentryLanesRolloverControl, 1, control_roll_scores1}},
	component_info{&control_attack_bump_tag, {BumperGroupControl, 0, nullptr}},
	component_info{&control_bump5_tag, {BumperControl, 4, control_bump_scores2}},
	component_info{&control_bump6_tag, {BumperControl, 4, control_bump_scores2}},
	component_info{&control_bump7_tag, {BumperControl, 4, control_bump_scores2}},
	component_info{&control_roll112_tag, {LaunchLanesRolloverControl, 1, control_roll_scores2}},
	component_info{&control_roll111_tag, {LaunchLanesRolloverControl, 1, control_roll_scores2}},
	component_info{&control_roll110_tag, {LaunchLanesRolloverControl, 1, control_roll_scores2}},
	component_info{&control_launch_bump_tag, {BumperGroupControl, 0, nullptr}},
	component_info{&control_rebo1_tag, {FlipperRebounderControl1, 1, control_rebo_score1}},
	component_info{&control_rebo2_tag, {FlipperRebounderControl2, 1, control_rebo_score1}},
	component_info{&control_rebo3_tag, {RebounderControl, 1, control_rebo_score1}},
	component_info{&control_rebo4_tag, {RebounderControl, 1, control_rebo_score1}},
	component_info{&control_kicker1_tag, {LeftKickerControl, 0, nullptr}},
	component_info{&control_kicker2_tag, {RightKickerControl, 0, nullptr}},
	component_info{&control_gate1_tag, {LeftKickerGateControl, 0, nullptr}},
	component_info{&control_gate2_tag, {RightKickerGateControl, 0, nullptr}},
	component_info{&control_oneway4_tag, {DeploymentChuteToEscapeChuteOneWayControl, 6, control_oneway4_score1}},
	component_info{&control_oneway10_tag, {DeploymentChuteToTableOneWayControl, 0, nullptr}},
	component_info{&control_block1_tag, {DrainBallBlockerControl, 0, nullptr}},
	component_info{&control_ramp_tag, {LaunchRampControl, 1, control_ramp_score1}},
	component_info{&control_ramp_hole_tag, {LaunchRampHoleControl, 0, nullptr}},
	component_info{&control_roll4_tag, {OutLaneRolloverControl, 1, control_roll_score1}},
	component_info{&control_roll8_tag, {OutLaneRolloverControl, 1, control_roll_score1}},
	component_info{&control_lite17_tag, {ExtraBallLightControl, 0, nullptr}},
	component_info{&control_roll6_tag, {ReturnLaneRolloverControl, 2, control_roll_score2}},
	component_info{&control_roll7_tag, {ReturnLaneRolloverControl, 2, control_roll_score2}},
	component_info{&control_roll5_tag, {BonusLaneRolloverControl, 1, control_roll_score3}},
	component_info{&control_roll179_tag, {FuelRollover1Control, 1, control_roll_score4}},
	component_info{&control_roll180_tag, {FuelRollover2Control, 1, control_roll_score4}},
	component_info{&control_roll181_tag, {FuelRollover3Control, 1, control_roll_score4}},
	component_info{&control_roll182_tag, {FuelRollover4Control, 1, control_roll_score4}},
	component_info{&control_roll183_tag, {FuelRollover5Control, 1, control_roll_score4}},
	component_info{&control_roll184_tag, {FuelRollover6Control, 1, control_roll_score4}},
	component_info{&control_flag1_tag, {FlagControl, 2, control_flag_score1}},
	component_info{&control_kickout2_tag, {HyperspaceKickOutControl, 5, control_kickout_score1}},
	component_info{&control_hyper_lights_tag, {HyperspaceLightGroupControl, 0, nullptr}},
	component_info{&control_flag2_tag, {FlagControl, 2, control_flag_score1}},
	component_info{&control_sink1_tag, {WormHoleControl, 3, control_sink_score1}},
	component_info{&control_sink2_tag, {WormHoleControl, 3, control_sink_score1}},
	component_info{&control_sink3_tag, {WormHoleControl, 3, control_sink_score1}},
	component_info{&control_flip1_tag, {LeftFlipperControl, 0, nullptr}},
	component_info{&control_flip2_tag, {RightFlipperControl, 0, nullptr}},
	component_info{&control_plunger_tag, {PlungerControl, 0, nullptr}},
	component_info{&control_target1_tag, {BoosterTargetControl, 2, control_target_score1}},
	component_info{&control_target2_tag, {BoosterTargetControl, 2, control_target_score1}},
	component_info{&control_target3_tag, {BoosterTargetControl, 2, control_target_score1}},
	component_info{&control_lite60_tag, {JackpotLightControl, 0, nullptr}},
	component_info{&control_lite59_tag, {BonusLightControl, 0, nullptr}},
	component_info{&control_target6_tag, {MedalTargetControl, 3, control_target_score2}},
	component_info{&control_target5_tag, {MedalTargetControl, 3, control_target_score2}},
	component_info{&control_target4_tag, {MedalTargetControl, 3, control_target_score2}},
	component_info{&control_bumber_target_lights_tag, {MedalLightGroupControl, 0, nullptr}},
	component_info{&control_target9_tag, {MultiplierTargetControl, 2, control_target_score3}},
	component_info{&control_target8_tag, {MultiplierTargetControl, 2, control_target_score3}},
	component_info{&control_target7_tag, {MultiplierTargetControl, 2, control_target_score3}},
	component_info{&control_top_target_lights_tag, {MultiplierLightGroupControl, 0, nullptr}},
	component_info{&control_target10_tag, {FuelSpotTargetControl, 1, control_target_score4}},
	component_info{&control_target11_tag, {FuelSpotTargetControl, 1, control_target_score4}},
	component_info{&control_target12_tag, {FuelSpotTargetControl, 1, control_target_score4}},
	component_info{&control_target13_tag, {MissionSpotTargetControl, 1, control_target_score5}},
	component_info{&control_target14_tag, {MissionSpotTargetControl, 1, control_target_score5}},
	component_info{&control_target15_tag, {MissionSpotTargetControl, 1, control_target_score5}},
	component_info{&control_target16_tag, {LeftHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target17_tag, {LeftHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target18_tag, {LeftHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target19_tag, {RightHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target20_tag, {RightHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target21_tag, {RightHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target22_tag, {WormHoleDestinationControl, 1, control_target_score7}},
	component_info{&control_roll9_tag, {SpaceWarpRolloverControl, 1, control_roll_score5}},
	component_info{&control_kickout3_tag, {BlackHoleKickoutControl, 1, control_kickout_score2}},
	component_info{&control_kickout1_tag, {GravityWellKickoutControl, 1, control_kickout_score3}},
	component_info{&control_drain_tag, {BallDrainControl, 0, nullptr}},
	component_info{&control_oneway1_tag, {SkillShotGate1Control, 0, nullptr}},
	component_info{&control_trip1_tag, {SkillShotGate2Control, 0, nullptr}},
	component_info{&control_trip2_tag, {SkillShotGate3Control, 0, nullptr}},
	component_info{&control_trip3_tag, {SkillShotGate4Control, 0, nullptr}},
	component_info{&control_trip4_tag, {SkillShotGate5Control, 0, nullptr}},
	component_info{&control_trip5_tag, {SkillShotGate6Control, 0, nullptr}},
	component_info{&control_lite200_tag, {ShootAgainLightControl, 0, nullptr}},
	component_info{&control_sink7_tag, {EscapeChuteSinkControl, 0, nullptr}},
};


component_tag* control::simple_components[142]
{
	&control_lite8_tag,
	&control_lite9_tag,
	&control_lite10_tag,
	&control_bmpr_inc_lights_tag,
	&control_lite171_tag,
	&control_lite170_tag,
	&control_lite169_tag,
	&control_ramp_bmpr_inc_lights_tag,
	&control_lite30_tag,
	&control_lite29_tag,
	&control_lite1_tag,
	&control_lite54_tag,
	&control_lite55_tag,
	&control_lite56_tag,
	&control_lite18_tag,
	&control_lite27_tag,
	&control_lite28_tag,
	&control_lite16_tag,
	&control_lite21_tag,
	&control_lite22_tag,
	&control_lite23_tag,
	&control_lite24_tag,
	&control_lite25_tag,
	&control_lite26_tag,
	&control_lite130_tag,
	&control_lite5_tag,
	&control_lite6_tag,
	&control_lite7_tag,
	&control_worm_hole_lights_tag,
	&control_lite4_tag,
	&control_lite2_tag,
	&control_lite3_tag,
	&control_bsink_arrow_lights_tag,
	&control_l_trek_lights_tag,
	&control_r_trek_lights_tag,
	&control_literoll179_tag,
	&control_literoll180_tag,
	&control_literoll181_tag,
	&control_literoll182_tag,
	&control_literoll183_tag,
	&control_literoll184_tag,
	&control_fuel_bargraph_tag,
	&control_lite20_tag,
	&control_lite19_tag,
	&control_lite61_tag,
	&control_lite58_tag,
	&control_lite11_tag,
	&control_lite12_tag,
	&control_lite13_tag,
	&control_lite70_tag,
	&control_lite71_tag,
	&control_lite72_tag,
	&control_top_circle_tgt_lights_tag,
	&control_lite101_tag,
	&control_lite102_tag,
	&control_lite103_tag,
	&control_ramp_tgt_lights_tag,
	&control_lite104_tag,
	&control_lite105_tag,
	&control_lite106_tag,
	&control_lite107_tag,
	&control_lite108_tag,
	&control_lite109_tag,
	&control_lchute_tgt_lights_tag,
	&control_bpr_solotgt_lights_tag,
	&control_lite110_tag,
	&control_lite62_tag,
	&control_lite67_tag,
	&control_lite68_tag,
	&control_lite69_tag,
	&control_lite131_tag,
	&control_lite132_tag,
	&control_lite133_tag,
	&control_skill_shot_lights_tag,
	&control_lite77_tag,
	&control_lite198_tag,
	&control_middle_circle_tag,
	&control_outer_circle_tag,
	&control_soundwave9_tag,
	&control_soundwave10_tag,
	&control_soundwave21_tag,
	&control_soundwave23_tag,
	&control_soundwave24_tag,
	&control_soundwave30_tag,
	&control_soundwave28_tag,
	&control_soundwave50_1_tag,
	&control_soundwave8_tag,
	&control_soundwave40_tag,
	&control_soundwave41_tag,
	&control_soundwave36_1_tag,
	&control_soundwave50_2_tag,
	&control_soundwave35_1_tag,
	&control_soundwave36_2_tag,
	&control_soundwave35_2_tag,
	&control_soundwave38_tag,
	&control_soundwave39_tag,
	&control_soundwave44_tag,
	&control_soundwave45_tag,
	&control_soundwave46_tag,
	&control_soundwave47_tag,
	&control_soundwave48_tag,
	&control_soundwave52_tag,
	&control_soundwave14_1_tag,
	&control_soundwave59_tag,
	&control_lite199_tag,
	&control_lite196_tag,
	&control_lite195_tag,
	&control_info_text_box_tag,
	&control_mission_text_box_tag,
	&control_soundwave27_tag,
	&control_lite84_tag,
	&control_lite85_tag,
	&control_soundwave14_2_tag,
	&control_soundwave3_tag,
	&control_soundwave26_tag,
	&control_soundwave49D_tag,
	&control_lite300_tag,
	&control_lite301_tag,
	&control_lite302_tag,
	&control_lite303_tag,
	&control_lite304_tag,
	&control_lite305_tag,
	&control_lite306_tag,
	&control_lite307_tag,
	&control_lite308_tag,
	&control_lite309_tag,
	&control_lite310_tag,
	&control_lite311_tag,
	&control_lite312_tag,
	&control_lite313_tag,
	&control_lite314_tag,
	&control_lite315_tag,
	&control_lite316_tag,
	&control_lite317_tag,
	&control_lite318_tag,
	&control_lite319_tag,
	&control_lite320_tag,
	&control_lite321_tag,
	&control_lite322_tag,
	&control_goal_lights_tag,
	&control_soundwave25_tag,
	&control_soundwave7_tag
};

int control::table_control_flag;


void control::make_links(TPinballTable* table)
{
	TableG = table;

	for (int index = 0; index < 88; index++)
	{
		auto compPtr = &score_components[index];
		TPinballComponent* comp = make_component_link(compPtr->Tag);
		if (comp)
		{
			comp->Control = &compPtr->Control;
			for (int scoreId = 0; scoreId < compPtr->Control.ScoreCount; scoreId++)
			{
				comp->put_scoring(scoreId, compPtr->Control.Scores[scoreId]);
			}
		}
	}

	for (int i = 0; i < 142; ++i)
		make_component_link(simple_components[i]);
}


TPinballComponent* control::make_component_link(component_tag* tag)
{
	if (tag->Component)
		return tag->Component;

	auto compList = TableG->ListP1;
	for (int index = 0; index < compList->Count(); index++)
	{
		auto comp = static_cast<TPinballComponent*>(compList->Get(index));
		if (comp->GroupName)
		{
			if (!strcmp(comp->GroupName, tag->Name))
			{
				tag->Component = comp;
				return comp;
			}
		}
	}

	return nullptr;
}

void control::handler(int code, TPinballComponent* cmp)
{
	component_control* control = cmp->Control;
	int scoreInd = 0;
	if (control)
	{
		if (code == 1019 && control->ScoreCount > 0)
		{
			do
			{
				cmp->put_scoring(scoreInd, control->Scores[scoreInd]);
				++scoreInd;
			}
			while (scoreInd < control->ScoreCount);
		}
		control->ControlFunc(code, cmp);
	}
	MissionControl(code, cmp);
}

void control::FlipperRebounderControl1(int code, TPinballComponent* caller)
{
}

void control::FlipperRebounderControl2(int code, TPinballComponent* caller)
{
}

void control::RebounderControl(int code, TPinballComponent* caller)
{
}

void control::BumperControl(int code, TPinballComponent* caller)
{
}

void control::LeftKickerControl(int code, TPinballComponent* caller)
{
}

void control::RightKickerControl(int code, TPinballComponent* caller)
{
}

void control::LeftKickerGateControl(int code, TPinballComponent* caller)
{
}

void control::RightKickerGateControl(int code, TPinballComponent* caller)
{
}

void control::DeploymentChuteToEscapeChuteOneWayControl(int code, TPinballComponent* caller)
{
}

void control::DeploymentChuteToTableOneWayControl(int code, TPinballComponent* caller)
{
}

void control::DrainBallBlockerControl(int code, TPinballComponent* caller)
{
}

void control::LaunchRampControl(int code, TPinballComponent* caller)
{
}

void control::LaunchRampHoleControl(int code, TPinballComponent* caller)
{
}

void control::SpaceWarpRolloverControl(int code, TPinballComponent* caller)
{
}

void control::ReentryLanesRolloverControl(int code, TPinballComponent* caller)
{
}

void control::BumperGroupControl(int code, TPinballComponent* caller)
{
}

void control::LaunchLanesRolloverControl(int code, TPinballComponent* caller)
{
}

void control::OutLaneRolloverControl(int code, TPinballComponent* caller)
{
}

void control::ExtraBallLightControl(int code, TPinballComponent* caller)
{
}

void control::ReturnLaneRolloverControl(int code, TPinballComponent* caller)
{
}

void control::BonusLaneRolloverControl(int code, TPinballComponent* caller)
{
}

void control::FuelRollover1Control(int code, TPinballComponent* caller)
{
}

void control::FuelRollover2Control(int code, TPinballComponent* caller)
{
}

void control::FuelRollover3Control(int code, TPinballComponent* caller)
{
}

void control::FuelRollover4Control(int code, TPinballComponent* caller)
{
}

void control::FuelRollover5Control(int code, TPinballComponent* caller)
{
}

void control::FuelRollover6Control(int code, TPinballComponent* caller)
{
}

void control::HyperspaceLightGroupControl(int code, TPinballComponent* caller)
{
}

void control::WormHoleControl(int code, TPinballComponent* caller)
{
}

void control::LeftFlipperControl(int code, TPinballComponent* caller)
{
}

void control::RightFlipperControl(int code, TPinballComponent* caller)
{
}

void control::JackpotLightControl(int code, TPinballComponent* caller)
{
}

void control::BonusLightControl(int code, TPinballComponent* caller)
{
}

void control::BoosterTargetControl(int code, TPinballComponent* caller)
{
}

void control::MedalLightGroupControl(int code, TPinballComponent* caller)
{
}

void control::MultiplierLightGroupControl(int code, TPinballComponent* caller)
{
}

void control::FuelSpotTargetControl(int code, TPinballComponent* caller)
{
}

void control::MissionSpotTargetControl(int code, TPinballComponent* caller)
{
}

void control::LeftHazardSpotTargetControl(int code, TPinballComponent* caller)
{
}

void control::RightHazardSpotTargetControl(int code, TPinballComponent* caller)
{
}

void control::WormHoleDestinationControl(int code, TPinballComponent* caller)
{
}

void control::BlackHoleKickoutControl(int code, TPinballComponent* caller)
{
}

void control::FlagControl(int code, TPinballComponent* caller)
{
}

void control::GravityWellKickoutControl(int code, TPinballComponent* caller)
{
}

void control::SkillShotGate1Control(int code, TPinballComponent* caller)
{
}

void control::SkillShotGate2Control(int code, TPinballComponent* caller)
{
}

void control::SkillShotGate3Control(int code, TPinballComponent* caller)
{
}

void control::SkillShotGate4Control(int code, TPinballComponent* caller)
{
}

void control::SkillShotGate5Control(int code, TPinballComponent* caller)
{
}

void control::SkillShotGate6Control(int code, TPinballComponent* caller)
{
}

void control::ShootAgainLightControl(int code, TPinballComponent* caller)
{
}

void control::EscapeChuteSinkControl(int code, TPinballComponent* caller)
{
}

void control::MissionControl(int code, TPinballComponent* caller)
{
}

void control::HyperspaceKickOutControl(int code, TPinballComponent* caller)
{
}

void control::PlungerControl(int code, TPinballComponent* caller)
{
}

void control::MedalTargetControl(int code, TPinballComponent* caller)
{
}

void control::MultiplierTargetControl(int code, TPinballComponent* caller)
{
}

void control::BallDrainControl(int code, TPinballComponent* caller)
{
}

void control::table_control_handler(int code)
{
	if (code == 1011)
	{
		table_control_flag = 0;
		control_lite77_tag.Component->Message(7, 0.0);
	}
}
