#include "pch.h"
#include "control.h"

#include "pb.h"
#include "pinball.h"
#include "TBlocker.h"
#include "TBumper.h"
#include "TComponentGroup.h"
#include "TFlagSpinner.h"
#include "TLight.h"
#include "TLightBargraph.h"
#include "TLightGroup.h"
#include "TLightRollover.h"
#include "TPinballTable.h"
#include "TPopupTarget.h"
#include "TSink.h"
#include "TSoloTarget.h"
#include "TSound.h"
#include "TTripwire.h"
#include "TDrain.h"
#include "TFlipper.h"
#include "TGate.h"
#include "THole.h"
#include "TKickback.h"
#include "TKickout.h"
#include "TOneway.h"
#include "TRamp.h"
#include "TPlunger.h"
#include "TWall.h"
#include "TTextBox.h"

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


component_tag<TComponentGroup> control_attack_bump_tag = {"attack_bumpers", nullptr};
component_tag<TComponentGroup> control_launch_bump_tag = {"launch_bumpers", nullptr};
component_tag<TBlocker> control_block1_tag = {"v_bloc1", nullptr};
component_tag<TBumper> control_bump1_tag = {"a_bump1", nullptr};
component_tag<TBumper> control_bump2_tag = {"a_bump2", nullptr};
component_tag<TBumper> control_bump3_tag = {"a_bump3", nullptr};
component_tag<TBumper> control_bump4_tag = {"a_bump4", nullptr};
component_tag<TBumper> control_bump5_tag = {"a_bump5", nullptr};
component_tag<TBumper> control_bump6_tag = {"a_bump6", nullptr};
component_tag<TBumper> control_bump7_tag = {"a_bump7", nullptr};
component_tag<TDrain> control_drain_tag = {"drain", nullptr};
component_tag<TFlagSpinner> control_flag1_tag = {"a_flag1", nullptr};
component_tag<TFlagSpinner> control_flag2_tag = {"a_flag2", nullptr};
component_tag<TFlipper> control_flip1_tag = {"a_flip1", nullptr};
component_tag<TFlipper> control_flip2_tag = {"a_flip2", nullptr};
component_tag<TLightBargraph> control_fuel_bargraph_tag = {"fuel_bargraph", nullptr};
component_tag<TGate> control_gate1_tag = {"v_gate1", nullptr};
component_tag<TGate> control_gate2_tag = {"v_gate2", nullptr};
component_tag<TTextBox> control_info_text_box_tag = {"info_text_box", nullptr};
component_tag<TKickback> control_kicker1_tag = {"a_kick1", nullptr};
component_tag<TKickback> control_kicker2_tag = {"a_kick2", nullptr};
component_tag<TKickout> control_kickout1_tag = {"a_kout1", nullptr};
component_tag<TKickout> control_kickout2_tag = {"a_kout2", nullptr};
component_tag<TKickout> control_kickout3_tag = {"a_kout3", nullptr};
component_tag<TLight> control_lite1_tag = {"lite1", nullptr};
component_tag<TLight> control_lite2_tag = {"lite2", nullptr};
component_tag<TLight> control_lite3_tag = {"lite3", nullptr};
component_tag<TLight> control_lite4_tag = {"lite4", nullptr};
component_tag<TLight> control_lite5_tag = {"lite5", nullptr};
component_tag<TLight> control_lite6_tag = {"lite6", nullptr};
component_tag<TLight> control_lite7_tag = {"lite7", nullptr};
component_tag<TLight> control_lite8_tag = {"lite8", nullptr};
component_tag<TLight> control_lite9_tag = {"lite9", nullptr};
component_tag<TLight> control_lite10_tag = {"lite10", nullptr};
component_tag<TLight> control_lite11_tag = {"lite11", nullptr};
component_tag<TLight> control_lite12_tag = {"lite12", nullptr};
component_tag<TLight> control_lite13_tag = {"lite13", nullptr};
component_tag<TLight> control_lite16_tag = {"lite16", nullptr};
component_tag<TLight> control_lite17_tag = {"lite17", nullptr};
component_tag<TLight> control_lite18_tag = {"lite18", nullptr};
component_tag<TLight> control_lite19_tag = {"lite19", nullptr};
component_tag<TLight> control_lite20_tag = {"lite20", nullptr};
component_tag<TLight> control_lite21_tag = {"lite21", nullptr};
component_tag<TLight> control_lite22_tag = {"lite22", nullptr};
component_tag<TLight> control_lite23_tag = {"lite23", nullptr};
component_tag<TLight> control_lite24_tag = {"lite24", nullptr};
component_tag<TLight> control_lite25_tag = {"lite25", nullptr};
component_tag<TLight> control_lite26_tag = {"lite26", nullptr};
component_tag<TLight> control_lite27_tag = {"lite27", nullptr};
component_tag<TLight> control_lite28_tag = {"lite28", nullptr};
component_tag<TLight> control_lite29_tag = {"lite29", nullptr};
component_tag<TLight> control_lite30_tag = {"lite30", nullptr};
component_tag<TLight> control_lite54_tag = {"lite54", nullptr};
component_tag<TLight> control_lite55_tag = {"lite55", nullptr};
component_tag<TLight> control_lite56_tag = {"lite56", nullptr};
component_tag<TLight> control_lite58_tag = {"lite58", nullptr};
component_tag<TLight> control_lite59_tag = {"lite59", nullptr};
component_tag<TLight> control_lite60_tag = {"lite60", nullptr};
component_tag<TLight> control_lite61_tag = {"lite61", nullptr};
component_tag<TLight> control_lite62_tag = {"lite62", nullptr};
component_tag<TLight> control_lite67_tag = {"lite67", nullptr};
component_tag<TLight> control_lite68_tag = {"lite68", nullptr};
component_tag<TLight> control_lite69_tag = {"lite69", nullptr};
component_tag<TLight> control_lite70_tag = {"lite70", nullptr};
component_tag<TLight> control_lite71_tag = {"lite71", nullptr};
component_tag<TLight> control_lite72_tag = {"lite72", nullptr};
component_tag<TLight> control_lite77_tag = {"lite77", nullptr};
component_tag<TLight> control_lite84_tag = {"lite84", nullptr};
component_tag<TLight> control_lite85_tag = {"lite85", nullptr};
component_tag<TLight> control_lite101_tag = {"lite101", nullptr};
component_tag<TLight> control_lite102_tag = {"lite102", nullptr};
component_tag<TLight> control_lite103_tag = {"lite103", nullptr};
component_tag<TLight> control_lite104_tag = {"lite104", nullptr};
component_tag<TLight> control_lite105_tag = {"lite105", nullptr};
component_tag<TLight> control_lite106_tag = {"lite106", nullptr};
component_tag<TLight> control_lite107_tag = {"lite107", nullptr};
component_tag<TLight> control_lite108_tag = {"lite108", nullptr};
component_tag<TLight> control_lite109_tag = {"lite109", nullptr};
component_tag<TLight> control_lite110_tag = {"lite110", nullptr};
component_tag<TLight> control_lite130_tag = {"lite130", nullptr};
component_tag<TLight> control_lite131_tag = {"lite131", nullptr};
component_tag<TLight> control_lite132_tag = {"lite132", nullptr};
component_tag<TLight> control_lite133_tag = {"lite133", nullptr};
component_tag<TLight> control_lite169_tag = {"lite169", nullptr};
component_tag<TLight> control_lite170_tag = {"lite170", nullptr};
component_tag<TLight> control_lite171_tag = {"lite171", nullptr};
component_tag<TLight> control_lite195_tag = {"lite195", nullptr};
component_tag<TLight> control_lite196_tag = {"lite196", nullptr};
component_tag<TLight> control_lite198_tag = {"lite198", nullptr};
component_tag<TLight> control_lite199_tag = {"lite199", nullptr};
component_tag<TLight> control_lite200_tag = {"lite200", nullptr};
component_tag<TLight> control_lite300_tag = {"lite300", nullptr};
component_tag<TLight> control_lite301_tag = {"lite301", nullptr};
component_tag<TLight> control_lite302_tag = {"lite302", nullptr};
component_tag<TLight> control_lite303_tag = {"lite303", nullptr};
component_tag<TLight> control_lite304_tag = {"lite304", nullptr};
component_tag<TLight> control_lite305_tag = {"lite305", nullptr};
component_tag<TLight> control_lite306_tag = {"lite306", nullptr};
component_tag<TLight> control_lite307_tag = {"lite307", nullptr};
component_tag<TLight> control_lite308_tag = {"lite308", nullptr};
component_tag<TLight> control_lite309_tag = {"lite309", nullptr};
component_tag<TLight> control_lite310_tag = {"lite310", nullptr};
component_tag<TLight> control_lite311_tag = {"lite311", nullptr};
component_tag<TLight> control_lite312_tag = {"lite312", nullptr};
component_tag<TLight> control_lite313_tag = {"lite313", nullptr};
component_tag<TLight> control_lite314_tag = {"lite314", nullptr};
component_tag<TLight> control_lite315_tag = {"lite315", nullptr};
component_tag<TLight> control_lite316_tag = {"lite316", nullptr};
component_tag<TLight> control_lite317_tag = {"lite317", nullptr};
component_tag<TLight> control_lite318_tag = {"lite318", nullptr};
component_tag<TLight> control_lite319_tag = {"lite319", nullptr};
component_tag<TLight> control_lite320_tag = {"lite320", nullptr};
component_tag<TLight> control_lite321_tag = {"lite321", nullptr};
component_tag<TLight> control_lite322_tag = {"lite322", nullptr};
component_tag<TLight> control_literoll179_tag = {"literoll179", nullptr};
component_tag<TLight> control_literoll180_tag = {"literoll180", nullptr};
component_tag<TLight> control_literoll181_tag = {"literoll181", nullptr};
component_tag<TLight> control_literoll182_tag = {"literoll182", nullptr};
component_tag<TLight> control_literoll183_tag = {"literoll183", nullptr};
component_tag<TLight> control_literoll184_tag = {"literoll184", nullptr};
component_tag<TLightGroup> control_middle_circle_tag = {"middle_circle", nullptr};
component_tag<TLightGroup> control_lchute_tgt_lights_tag = {"lchute_tgt_lights", nullptr};
component_tag<TLightGroup> control_l_trek_lights_tag = {"l_trek_lights", nullptr};
component_tag<TLightGroup> control_goal_lights_tag = {"goal_lights", nullptr};
component_tag<TLightGroup> control_hyper_lights_tag = {"hyperspace_lights", nullptr};
component_tag<TLightGroup> control_bmpr_inc_lights_tag = {"bmpr_inc_lights", nullptr};
component_tag<TLightGroup> control_bpr_solotgt_lights_tag = {"bpr_solotgt_lights", nullptr};
component_tag<TLightGroup> control_bsink_arrow_lights_tag = {"bsink_arrow_lights", nullptr};
component_tag<TLightGroup> control_bumber_target_lights_tag = {"bumper_target_lights", nullptr};
component_tag<TLightGroup> control_outer_circle_tag = {"outer_circle", nullptr};
component_tag<TLightGroup> control_r_trek_lights_tag = {"r_trek_lights", nullptr};
component_tag<TLightGroup> control_ramp_bmpr_inc_lights_tag = {"ramp_bmpr_inc_lights", nullptr};
component_tag<TLightGroup> control_ramp_tgt_lights_tag = {"ramp_tgt_lights", nullptr};
component_tag<TLightGroup> control_skill_shot_lights_tag = {"skill_shot_lights", nullptr};
component_tag<TLightGroup> control_top_circle_tgt_lights_tag = {"top_circle_tgt_lights", nullptr};
component_tag<TLightGroup> control_top_target_lights_tag = {"top_target_lights", nullptr};
component_tag<TLightGroup> control_worm_hole_lights_tag = {"worm_hole_lights", nullptr};
component_tag<TTextBox> control_mission_text_box_tag = {"mission_text_box", nullptr};
component_tag<TOneway> control_oneway1_tag = {"s_onewy1", nullptr};
component_tag<TOneway> control_oneway4_tag = {"s_onewy4", nullptr};
component_tag<TOneway> control_oneway10_tag = {"s_onewy10", nullptr};
component_tag<TPlunger> control_plunger_tag = {"plunger", nullptr};
component_tag<THole> control_ramp_hole_tag = {"ramp_hole", nullptr};
component_tag<TRamp> control_ramp_tag = {"ramp", nullptr};
component_tag<TWall> control_rebo1_tag = {"v_rebo1", nullptr};
component_tag<TWall> control_rebo2_tag = {"v_rebo2", nullptr};
component_tag<TWall> control_rebo3_tag = {"v_rebo3", nullptr};
component_tag<TWall> control_rebo4_tag = {"v_rebo4", nullptr};
component_tag<TRollover> control_roll1_tag = {"a_roll1", nullptr};
component_tag<TRollover> control_roll2_tag = {"a_roll2", nullptr};
component_tag<TRollover> control_roll3_tag = {"a_roll3", nullptr};
component_tag<TRollover> control_roll4_tag = {"a_roll4", nullptr};
component_tag<TRollover> control_roll5_tag = {"a_roll5", nullptr};
component_tag<TRollover> control_roll6_tag = {"a_roll6", nullptr};
component_tag<TRollover> control_roll7_tag = {"a_roll7", nullptr};
component_tag<TRollover> control_roll8_tag = {"a_roll8", nullptr};
component_tag<TLightRollover> control_roll9_tag = {"a_roll9", nullptr};
component_tag<TRollover> control_roll110_tag = {"a_roll110", nullptr};
component_tag<TRollover> control_roll111_tag = {"a_roll111", nullptr};
component_tag<TRollover> control_roll112_tag = {"a_roll112", nullptr};
component_tag<TRollover> control_roll179_tag = {"a_roll179", nullptr};
component_tag<TRollover> control_roll180_tag = {"a_roll180", nullptr};
component_tag<TRollover> control_roll181_tag = {"a_roll181", nullptr};
component_tag<TRollover> control_roll182_tag = {"a_roll182", nullptr};
component_tag<TRollover> control_roll183_tag = {"a_roll183", nullptr};
component_tag<TRollover> control_roll184_tag = {"a_roll184", nullptr};
component_tag<TSink> control_sink1_tag = {"v_sink1", nullptr};
component_tag<TSink> control_sink2_tag = {"v_sink2", nullptr};
component_tag<TSink> control_sink3_tag = {"v_sink3", nullptr};
component_tag<TSink> control_sink7_tag = {"v_sink7", nullptr};
component_tag<TSound> control_soundwave3_tag = {"soundwave3", nullptr};
component_tag<TSound> control_soundwave7_tag = {"soundwave7", nullptr};
component_tag<TSound> control_soundwave8_tag = {"soundwave8", nullptr};
component_tag<TSound> control_soundwave9_tag = {"soundwave9", nullptr};
component_tag<TSound> control_soundwave10_tag = {"soundwave10", nullptr};
component_tag<TSound> control_soundwave14_1_tag = {"soundwave14", nullptr};
component_tag<TSound> control_soundwave14_2_tag = {"soundwave14", nullptr};
component_tag<TSound> control_soundwave21_tag = {"soundwave21", nullptr};
component_tag<TSound> control_soundwave23_tag = {"soundwave23", nullptr};
component_tag<TSound> control_soundwave24_tag = {"soundwave24", nullptr};
component_tag<TSound> control_soundwave25_tag = {"soundwave25", nullptr};
component_tag<TSound> control_soundwave26_tag = {"soundwave26", nullptr};
component_tag<TSound> control_soundwave27_tag = {"soundwave27", nullptr};
component_tag<TSound> control_soundwave28_tag = {"soundwave28", nullptr};
component_tag<TSound> control_soundwave30_tag = {"soundwave30", nullptr};
component_tag<TSound> control_soundwave35_1_tag = {"soundwave35", nullptr};
component_tag<TSound> control_soundwave35_2_tag = {"soundwave35", nullptr};
component_tag<TSound> control_soundwave36_1_tag = {"soundwave36", nullptr};
component_tag<TSound> control_soundwave36_2_tag = {"soundwave36", nullptr};
component_tag<TSound> control_soundwave38_tag = {"soundwave38", nullptr};
component_tag<TSound> control_soundwave39_tag = {"soundwave39", nullptr};
component_tag<TSound> control_soundwave40_tag = {"soundwave40", nullptr};
component_tag<TSound> control_soundwave41_tag = {"soundwave41", nullptr};
component_tag<TSound> control_soundwave44_tag = {"soundwave44", nullptr};
component_tag<TSound> control_soundwave45_tag = {"soundwave45", nullptr};
component_tag<TSound> control_soundwave46_tag = {"soundwave46", nullptr};
component_tag<TSound> control_soundwave47_tag = {"soundwave47", nullptr};
component_tag<TSound> control_soundwave48_tag = {"soundwave48", nullptr};
component_tag<TSound> control_soundwave49D_tag = {"soundwave49D", nullptr};
component_tag<TSound> control_soundwave50_1_tag = {"soundwave50", nullptr};
component_tag<TSound> control_soundwave50_2_tag = {"soundwave50", nullptr};
component_tag<TSound> control_soundwave52_tag = {"soundwave52", nullptr};
component_tag<TSound> control_soundwave59_tag = {"soundwave59", nullptr};
component_tag<TPopupTarget> control_target1_tag = {"a_targ1", nullptr};
component_tag<TPopupTarget> control_target2_tag = {"a_targ2", nullptr};
component_tag<TPopupTarget> control_target3_tag = {"a_targ3", nullptr};
component_tag<TPopupTarget> control_target4_tag = {"a_targ4", nullptr};
component_tag<TPopupTarget> control_target5_tag = {"a_targ5", nullptr};
component_tag<TPopupTarget> control_target6_tag = {"a_targ6", nullptr};
component_tag<TPopupTarget> control_target7_tag = {"a_targ7", nullptr};
component_tag<TPopupTarget> control_target8_tag = {"a_targ8", nullptr};
component_tag<TPopupTarget> control_target9_tag = {"a_targ9", nullptr};
component_tag<TSoloTarget> control_target10_tag = {"a_targ10", nullptr};
component_tag<TSoloTarget> control_target11_tag = {"a_targ11", nullptr};
component_tag<TSoloTarget> control_target12_tag = {"a_targ12", nullptr};
component_tag<TSoloTarget> control_target13_tag = {"a_targ13", nullptr};
component_tag<TSoloTarget> control_target14_tag = {"a_targ14", nullptr};
component_tag<TSoloTarget> control_target15_tag = {"a_targ15", nullptr};
component_tag<TSoloTarget> control_target16_tag = {"a_targ16", nullptr};
component_tag<TSoloTarget> control_target17_tag = {"a_targ17", nullptr};
component_tag<TSoloTarget> control_target18_tag = {"a_targ18", nullptr};
component_tag<TSoloTarget> control_target19_tag = {"a_targ19", nullptr};
component_tag<TSoloTarget> control_target20_tag = {"a_targ20", nullptr};
component_tag<TSoloTarget> control_target21_tag = {"a_targ21", nullptr};
component_tag<TSoloTarget> control_target22_tag = {"a_targ22", nullptr};
component_tag<TTripwire> control_trip1_tag = {"s_trip1", nullptr};
component_tag<TTripwire> control_trip2_tag = {"s_trip2", nullptr};
component_tag<TTripwire> control_trip3_tag = {"s_trip3", nullptr};
component_tag<TTripwire> control_trip4_tag = {"s_trip4", nullptr};
component_tag<TTripwire> control_trip5_tag = {"s_trip5", nullptr};


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


component_tag_base* control::simple_components[142]
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

int control::waiting_deployment_flag;
bool control::table_unlimited_balls = false;
int control::extraball_light_flag;
int control::RankRcArray[9] = {84, 85, 86, 87, 88, 89, 90, 91, 92};
int control::MissionRcArray[17] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};
int control::mission_select_scores[17] =
{
	10000,
	10000,
	10000,
	10000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	30000,
	30000,
	30000,
	30000
};
component_tag_base* control::wormhole_tag_array1[3] =
{
	&control_sink1_tag, &control_sink2_tag, &control_sink3_tag
};

component_tag_base* control::wormhole_tag_array2[3] =
{
	&control_lite5_tag, &control_lite6_tag, &control_lite7_tag
};

component_tag_base* control::wormhole_tag_array3[3] =
{
	&control_lite4_tag, &control_lite2_tag, &control_lite3_tag
};


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

void control::ClearLinks()
{
	TableG = nullptr;
	for (auto& component : score_components)
		component.Tag->SetComponent(nullptr);
	for (auto& component : simple_components)
		component->SetComponent(nullptr);
}

TPinballComponent* control::make_component_link(component_tag_base* tag)
{
	if (tag->GetComponent())
		return tag->GetComponent();

	for (auto component: TableG->ComponentList)
	{
		if (component->GroupName)
		{
			if (!strcmp(component->GroupName, tag->Name))
			{
				tag->SetComponent(component);
				return component;
			}
		}
	}

	return nullptr;
}

void control::handler(int code, TPinballComponent* cmp)
{
	component_control* control = cmp->Control;
	
	if (control)
	{
		if (code == 1019)
		{
			for (auto scoreInd = 0; scoreInd < control->ScoreCount; ++scoreInd)
			{
				cmp->put_scoring(scoreInd, control->Scores[scoreInd]);
			}
		}
		control->ControlFunc(code, cmp);
	}
	MissionControl(code, cmp);
}

void control::pbctrl_bdoor_controller(char key)
{
	// Buffer large enough for longest cheat + null
	static char cheatBuffer[11 + 1]{};
	static char* bufferEnd = &cheatBuffer[11];
	static const char* quotes[8]
	{
		"Hey, is that a screen saver?",
		"I guess it has been a good week",
		"She may already be a glue bottle",
		"If you don't come in Saturday,\n...\n",
		"don't even bother coming in Sunday.",
		"Tomorrow already sucks",
		"I knew it worked too good to be right.",
		"World's most expensive flippers"
	};

	// Original allowed to enter cheats only before the first launch.
	std::memmove(&cheatBuffer[0], &cheatBuffer[1], 10);
	cheatBuffer[10] = key;

	if (strcmp(bufferEnd - 11, "hidden test") == 0)
	{
		pb::cheat_mode ^= true;
	}
	else if (strcmp(bufferEnd - 4, "gmax") == 0)
	{
		GravityWellKickoutControl(64, nullptr);
	}
	else if (strcmp(bufferEnd - 4, "1max") == 0)
	{
		table_add_extra_ball(2.0);
	}
	else if (strcmp(bufferEnd - 4, "bmax") == 0)
	{
		table_unlimited_balls ^= true;
	}
	else if (strcmp(bufferEnd - 4, "rmax") == 0)
	{
		cheat_bump_rank();
	}
	else if (pb::FullTiltMode && strcmp(bufferEnd - 5, "quote") == 0)
	{
		// A sad developer easter egg type 'cheat' from Full Tilt 
		float time = 0;
		for (auto quote : quotes)
			control_mission_text_box_tag.Component->Display(quote, time += 3);
		return;
	}
	else
	{
		return;
	}

	TableG->CheatsUsed = 1;
}

void control::table_add_extra_ball(float count)
{
	++TableG->ExtraBalls;
	control_soundwave28_tag.Component->Play();
	auto msg = pinball::get_rc_string(9, 0);
	control_info_text_box_tag.Component->Display(msg, count);
}

void control::table_set_bonus_hold()
{
	control_lite58_tag.Component->Message(19, 0.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(52, 0), 2.0);
}

void control::table_set_bonus()
{
	TableG->ScoreSpecial2Flag = 1;
	control_lite59_tag.Component->Message(9, 60.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(4, 0), 2.0);
}

void control::table_set_jackpot()
{
	TableG->ScoreSpecial3Flag = 1;
	control_lite60_tag.Component->Message(9, 60.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(15, 0), 2.0);
}

void control::table_set_flag_lights()
{
	control_lite20_tag.Component->Message(9, 60.0);
	control_lite19_tag.Component->Message(9, 60.0);
	control_lite61_tag.Component->Message(9, 60.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(51, 0), 2.0);
}

void control::table_set_multiball()
{
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(16, 0), 2.0);
}

void control::table_bump_ball_sink_lock()
{
	if (TableG->BallLockedCounter == 2)
	{
		table_set_multiball();
		TableG->BallLockedCounter = 0;
	}
	else
	{
		TableG->BallLockedCounter = TableG->BallLockedCounter + 1;
		control_soundwave44_tag.Component->Play();
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(1, 0), 2.0);
		TableG->Plunger->Message(1016, 0.0);
	}
}

void control::table_set_replay(float value)
{
	control_lite199_tag.Component->Message(19, 0.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(0, 0), value);
}

void control::cheat_bump_rank()
{
	char Buffer[64]{};

	auto rank = control_middle_circle_tag.Component->Message(37, 0.0);
	if (rank < 9)
	{
		control_middle_circle_tag.Component->Message(41, 2.0f);
		auto rankText = pinball::get_rc_string(RankRcArray[rank], 1);
		snprintf(Buffer,sizeof Buffer, pinball::get_rc_string(83, 0), rankText);
		control_mission_text_box_tag.Component->Display(Buffer, 8.0);
		control_soundwave10_tag.Component->Play();
	}
}

bool control::light_on(component_tag<TLight>* tag)
{
	auto light = tag->Component;
	return light->BmpIndex1 || light->FlasherFlag2 || light->FlasherActive;
}

int control::SpecialAddScore(int score)
{
	int prevFlag1 = TableG->ScoreSpecial3Flag;
	TableG->ScoreSpecial3Flag = 0;
	int prevFlag2 = TableG->ScoreSpecial2Flag;
	TableG->ScoreSpecial2Flag = 0;
	int prevMult = TableG->ScoreMultiplier;
	TableG->ScoreMultiplier = 0;

	int addedScore = TableG->AddScore(score);
	TableG->ScoreSpecial2Flag = prevFlag2;
	TableG->ScoreMultiplier = prevMult;
	TableG->ScoreSpecial3Flag = prevFlag1;
	return addedScore;
}

int control::AddRankProgress(int rank)
{
	char Buffer[64];
	int result = 0;

	control_lite16_tag.Component->Message(19, 0.0);
	TPinballComponent* outerCircle = control_outer_circle_tag.Component;
	for (int index = rank; index; --index)
	{
		outerCircle->Message(41, 2.0);
	}

	int activeCount = outerCircle->Message(37, 0.0);
	int totalCount = outerCircle->Message(38, 0.0);
	if (activeCount == totalCount)
	{
		result = 1;
		outerCircle->Message(16, 5.0);
		TPinballComponent* middleCircle = control_middle_circle_tag.Component;
		control_middle_circle_tag.Component->Message(34, 0.0);
		int midActiveCount = middleCircle->Message(37, 0.0);
		if (midActiveCount < 9)
		{
			middleCircle->Message(41, 5.0);
			auto rankText = pinball::get_rc_string(RankRcArray[midActiveCount], 1);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(83, 0), rankText);
			control_mission_text_box_tag.Component->Display(Buffer, 8.0);
			control_soundwave10_tag.Component->Play();
		}
	}
	else if (activeCount >= 3 * totalCount / 4)
	{
		control_middle_circle_tag.Component->Message(27, -1.0);
	}
	return result;
}

void control::AdvanceWormHoleDestination(int flag)
{
	int lite198Msg = control_lite198_tag.Component->MessageField;
	if (lite198Msg != 16 && lite198Msg != 22 && lite198Msg != 23)
	{
		int lite4Msg = control_lite4_tag.Component->MessageField;
		if (flag || lite4Msg)
		{
			int val1 = lite4Msg + 1;
			int val2 = val1;
			if (val1 == 4)
			{
				val1 = 1;
				val2 = 1;
			}
			control_bsink_arrow_lights_tag.Component->Message(23, static_cast<float>(val2));
			control_bsink_arrow_lights_tag.Component->Message(11, static_cast<float>(3 - val1));
			if (!light_on(&control_lite4_tag))
			{
				control_worm_hole_lights_tag.Component->Message(19, 0.0);
				control_bsink_arrow_lights_tag.Component->Message(19, 0.0);
			}
		}
	}
}

void control::FlipperRebounderControl1(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite84_tag.Component->Message(9, 0.1f);
		auto score = caller->get_scoring(0);
		TableG->AddScore(score);
	}
}

void control::FlipperRebounderControl2(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite85_tag.Component->Message(9, 0.1f);
		int score = caller->get_scoring(0);
		TableG->AddScore(score);
	}
}

void control::RebounderControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::BumperControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		TableG->AddScore(caller->get_scoring(static_cast<TBumper*>(caller)->BmpIndex));
	}
}

void control::LeftKickerControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		control_gate1_tag.Component->Message(54, 0.0);
}

void control::RightKickerControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		control_gate2_tag.Component->Message(54, 0.0);
}

void control::LeftKickerGateControl(int code, TPinballComponent* caller)
{
	if (code == 53)
	{
		control_lite30_tag.Component->Message(15, 5.0);
		control_lite196_tag.Component->Message(7, 5.0);
	}
	else if (code == 54)
	{
		control_lite30_tag.Component->Message(20, 0.0);
		control_lite196_tag.Component->Message(20, 0.0);
	}
}

void control::RightKickerGateControl(int code, TPinballComponent* caller)
{
	if (code == 53)
	{
		control_lite29_tag.Component->Message(15, 5.0);
		control_lite195_tag.Component->Message(7, 5.0);
	}
	else if (code == 54)
	{
		control_lite29_tag.Component->Message(20, 0.0);
		control_lite195_tag.Component->Message(20, 0.0);
	}
}

void control::DeploymentChuteToEscapeChuteOneWayControl(int code, TPinballComponent* caller)
{
	char Buffer[64];
	if (code == 63)
	{
		int count = control_skill_shot_lights_tag.Component->Message(37, 0.0);
		if (count)
		{
			control_soundwave3_tag.Component->Play();
			int score = TableG->AddScore(caller->get_scoring(count - 1));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(21, 0), score);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			if (!light_on(&control_lite56_tag))
			{
				control_l_trek_lights_tag.Component->Message(34, 0.0);
				control_l_trek_lights_tag.Component->Message(20, 0.0);
				control_r_trek_lights_tag.Component->Message(34, 0.0);
				control_r_trek_lights_tag.Component->Message(20, 0.0);
			}
			control_skill_shot_lights_tag.Component->Message(44, 1.0);
		}
	}
}

void control::DeploymentChuteToTableOneWayControl(int code, TPinballComponent* caller)
{
	if (code == 63)
		control_skill_shot_lights_tag.Component->Message(20, 0.0);
}

void control::DrainBallBlockerControl(int code, TPinballComponent* caller)
{
	int msgCode;
	float msgValue;

	auto block = static_cast<TBlocker*>(caller);
	if (code == 52)
	{
		block->MessageField = 1;
		block->Message(52, static_cast<float>(block->TurnOnMsgValue));
		msgValue = static_cast<float>(block->TurnOnMsgValue);
		msgCode = 9;
	}
	else
	{
		if (code != 60)
			return;
		if (block->MessageField != 1)
		{
			block->MessageField = 0;
			block->Message(51, 0.0);
			return;
		}
		block->MessageField = 2;
		block->Message(59, static_cast<float>(block->TurnOffMsgValue));
		msgValue = static_cast<float>(block->TurnOffMsgValue);
		msgCode = 7;
	}
	control_lite1_tag.Component->Message(msgCode, msgValue);
}

void control::LaunchRampControl(int code, TPinballComponent* caller)
{
	TSound* sound;
	char Buffer[64];

	if (code == 63)
	{
		int someFlag = 0;
		if (light_on(&control_lite54_tag))
		{
			someFlag = 1;
			int addedScore = SpecialAddScore(TableG->ScoreSpecial1);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(10, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
		}
		if (light_on(&control_lite55_tag))
			someFlag |= 2u;
		if (light_on(&control_lite56_tag))
			someFlag |= 4u;
		if (someFlag)
		{
			if (someFlag == 1)
			{
				sound = control_soundwave21_tag.Component;
			}
			else if (someFlag < 1 || someFlag > 3)
			{
				sound = control_soundwave24_tag.Component;
			}
			else
			{
				sound = control_soundwave23_tag.Component;
			}
		}
		else
		{
			TableG->AddScore(caller->get_scoring(0));
			sound = control_soundwave30_tag.Component;
		}
		sound->Play();
	}
}

void control::LaunchRampHoleControl(int code, TPinballComponent* caller)
{
	if (code == 58)
		control_lite54_tag.Component->Message(7, 5.0);
}

void control::SpaceWarpRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite27_tag.Component->Message(19, 0.0);
		control_lite28_tag.Component->Message(19, 0.0);
	}
}

void control::ReentryLanesRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (!light_on(&control_lite56_tag) && control_l_trek_lights_tag.Component->Message(39, 0.0))
		{
			control_l_trek_lights_tag.Component->Message(34, 0.0);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(34, 0.0);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
		}

		TLight* light;
		if (control_roll3_tag.Component == caller)
		{
			light = control_lite8_tag.Component;
		}
		else
		{
			light = control_lite9_tag.Component;
			if (control_roll2_tag.Component != caller)
				light = control_lite10_tag.Component;
		}
		if (!light->FlasherActive)
		{
			if (light->BmpIndex1)
			{
				light->Message(20, 0.0);
			}
			else
			{
				light->Message(19, 0.0);
				int activeCount = control_bmpr_inc_lights_tag.Component->Message(37, 0.0);
				if (activeCount == control_bmpr_inc_lights_tag.Component->Message(38, 0.0))
				{
					control_bmpr_inc_lights_tag.Component->Message(7, 5.0);
					control_bmpr_inc_lights_tag.Component->Message(0, 0.0);
					if (control_bump1_tag.Component->BmpIndex < 3)
					{
						control_attack_bump_tag.Component->Message(12, 0.0);
						control_info_text_box_tag.Component->Display(pinball::get_rc_string(5, 0), 2.0);
					}
					control_attack_bump_tag.Component->Message(48, 60.0);
				}
			}
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::BumperGroupControl(int code, TPinballComponent* caller)
{
	if (code == 61)
	{
		/*Bug in the original. Caller (TComponentGroup) is accessed beyond bounds at 0x4E*/
		if (static_cast<TBumper*>(caller)->BmpIndex)
		{
			caller->Message(48, 60.0);
			caller->Message(13, 0.0);
		}
	}
}

void control::LaunchLanesRolloverControl(int code, TPinballComponent* caller)
{
	TLight* light;

	if (code == 63)
	{
		if (control_roll112_tag.Component == caller)
		{
			light = control_lite171_tag.Component;
		}
		else
		{
			light = control_lite170_tag.Component;
			if (control_roll111_tag.Component != caller)
				light = control_lite169_tag.Component;
		}
		if (!light->FlasherActive)
		{
			if (light->BmpIndex1)
			{
				light->Message(20, 0.0);
			}
			else
			{
				light->Message(19, 0.0);
				int msg1 = control_ramp_bmpr_inc_lights_tag.Component->Message(37, 0.0);
				if (msg1 == control_ramp_bmpr_inc_lights_tag.Component->Message(38, 0.0))
				{
					control_ramp_bmpr_inc_lights_tag.Component->Message(7, 5.0);
					control_ramp_bmpr_inc_lights_tag.Component->Message(0, 0.0);
					if (control_bump5_tag.Component->BmpIndex < 3)
					{
						control_launch_bump_tag.Component->Message(12, 0.0);
						control_info_text_box_tag.Component->Display(pinball::get_rc_string(6, 0), 2.0);
					}
					control_launch_bump_tag.Component->Message(48, 60.0);
				}
			}
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::OutLaneRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite17_tag) || light_on(&control_lite18_tag))
		{
			table_add_extra_ball(2.0);
			control_lite17_tag.Component->Message(20, 0.0);
			control_lite18_tag.Component->Message(20, 0.0);
		}
		else
		{
			control_soundwave26_tag.Component->Play();
		}
		if (control_roll4_tag.Component == caller)
		{
			if (light_on(&control_lite30_tag))
			{
				control_lite30_tag.Component->Message(4, 0.0);
				control_lite196_tag.Component->Message(4, 0.0);
			}
		}
		else if (light_on(&control_lite29_tag))
		{
			control_lite29_tag.Component->Message(4, 0.0);
			control_lite195_tag.Component->Message(4, 0.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::ExtraBallLightControl(int code, TPinballComponent* caller)
{
	if (code == 19)
	{
		control_lite17_tag.Component->Message(9, 55.0);
		control_lite18_tag.Component->Message(9, 55.0);
		extraball_light_flag = 1;
	}
	else if (code == 60)
	{
		if (extraball_light_flag)
		{
			control_lite17_tag.Component->Message(7, 5.0);
			control_lite18_tag.Component->Message(7, 5.0);
			extraball_light_flag = 0;
		}
	}
}

void control::ReturnLaneRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_roll6_tag.Component == caller)
		{
			if (light_on(&control_lite27_tag))
			{
				control_lite59_tag.Component->Message(20, 0.0);
				control_lite27_tag.Component->Message(20, 0.0);
				TableG->AddScore(caller->get_scoring(1));
			}
			else
				TableG->AddScore(caller->get_scoring(0));
		}
		else if (control_roll7_tag.Component == caller)
		{
			if (light_on(&control_lite28_tag))
			{
				control_lite59_tag.Component->Message(20, 0.0);
				control_lite28_tag.Component->Message(20, 0.0);
				TableG->AddScore(caller->get_scoring(1));
			}
			else
				TableG->AddScore(caller->get_scoring(0));
		}
	}
}

void control::BonusLaneRolloverControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		if (light_on(&control_lite16_tag))
		{
			int addedScore = SpecialAddScore(TableG->ScoreSpecial2);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(3, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			control_lite16_tag.Component->Message(20, 0.0);
			control_soundwave50_1_tag.Component->Play();
		}
		else
		{
			TableG->AddScore(caller->get_scoring(0));
			control_soundwave25_tag.Component->Play();
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		control_fuel_bargraph_tag.Component->Message(45, 11.0);
	}
}

void control::FuelRollover1Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 1)
		{
			control_literoll179_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 1.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover2Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 3)
		{
			control_literoll180_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 3.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover3Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 5)
		{
			control_literoll181_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 5.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover4Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 7)
		{
			control_literoll182_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 7.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover5Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 9)
		{
			control_literoll183_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 9.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover6Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 11)
		{
			control_literoll184_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 11.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::HyperspaceLightGroupControl(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 0:
		caller->Message(0, 0.0);
		break;
	case 41:
		caller->Message(41, 2.0);
		caller->Message(43, 60.0);
		break;
	case 61:
		caller->Message(33, 0.0);
		if (caller->Message(37, 0.0))
			caller->Message(43, 60.0);
		break;
	default: break;
	}
}

void control::WormHoleControl(int code, TPinballComponent* caller)
{
	int sinkFlag2;
	TSink* sink = static_cast<TSink*>(caller);

	if (code == 63)
	{
		int sinkFlag = 0;
		if (control_sink1_tag.Component != sink)
		{
			sinkFlag = control_sink2_tag.Component != sink;
			++sinkFlag;
		}

		int lite4Msg = control_lite4_tag.Component->MessageField;
		if (lite4Msg)
		{
			control_lite4_tag.Component->MessageField = 0;
			control_worm_hole_lights_tag.Component->Message(20, 0.0);
			control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
			control_lite110_tag.Component->Message(20, 0.0);
			if (lite4Msg == sinkFlag + 1)
			{
				if (TableG->MultiballFlag)
				{
					table_bump_ball_sink_lock();
					TableG->AddScore(10000);
				}
				else
				{
					control_info_text_box_tag.Component->Display(pinball::get_rc_string(49, 0), 2.0);
					table_set_replay(4.0);
					TableG->AddScore(sink->get_scoring(1));
					wormhole_tag_array2[sinkFlag]->GetComponent()->Message(16, sink->TimerTime);
					wormhole_tag_array3[sinkFlag]->GetComponent()->Message(11, static_cast<float>(2 - sinkFlag));
					wormhole_tag_array3[sinkFlag]->GetComponent()->Message(16, sink->TimerTime);
					wormhole_tag_array1[sinkFlag]->GetComponent()->Message(56, sink->TimerTime);
				}
				return;
			}
			TableG->AddScore(sink->get_scoring(2));
			sinkFlag2 = lite4Msg - 1;
		}
		else
		{
			TableG->AddScore(sink->get_scoring(0));
			sinkFlag2 = sinkFlag;
		}

		wormhole_tag_array2[sinkFlag2]->GetComponent()->Message(16, sink->TimerTime);
		wormhole_tag_array3[sinkFlag2]->GetComponent()->Message(11, static_cast<float>(2 - sinkFlag2));
		wormhole_tag_array3[sinkFlag2]->GetComponent()->Message(16, sink->TimerTime);
		wormhole_tag_array1[sinkFlag2]->GetComponent()->Message(56, sink->TimerTime);
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(49, 0), 2.0);
	}
}

void control::LeftFlipperControl(int code, TPinballComponent* caller)
{
	if (code == 1)
	{
		control_bmpr_inc_lights_tag.Component->Message(24, 0.0);
		control_ramp_bmpr_inc_lights_tag.Component->Message(24, 0.0);
	}
}

void control::RightFlipperControl(int code, TPinballComponent* caller)
{
	if (code == 1)
	{
		control_bmpr_inc_lights_tag.Component->Message(25, 0.0);
		control_ramp_bmpr_inc_lights_tag.Component->Message(25, 0.0);
	}
}

void control::JackpotLightControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		TableG->ScoreSpecial3Flag = 0;
}

void control::BonusLightControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		TableG->ScoreSpecial2Flag = 0;
}

void control::BoosterTargetControl(int code, TPinballComponent* caller)
{
	TSound* sound = nullptr;

	if (code == 63 && !caller->MessageField)
	{
		caller->MessageField = 1;
		if (control_target1_tag.Component->MessageField
			+ control_target2_tag.Component->MessageField
			+ control_target3_tag.Component->MessageField != 3)
		{
			TableG->AddScore(caller->get_scoring(0));
			return;
		}
		if (light_on(&control_lite61_tag))
		{
			if (light_on(&control_lite60_tag))
			{
				if (light_on(&control_lite59_tag))
				{
					if (light_on(&control_lite58_tag))
					{
						TableG->AddScore(caller->get_scoring(1));
					}
					else
					{
						table_set_bonus_hold();
					}
					sound = control_soundwave48_tag.Component;
				}
				else
				{
					table_set_bonus();
					sound = control_soundwave46_tag.Component;
				}
			}
			else
			{
				table_set_jackpot();
				sound = control_soundwave45_tag.Component;
			}
		}
		else
		{
			int msg = control_lite198_tag.Component->MessageField;
			if (msg != 15 && msg != 29)
			{
				table_set_flag_lights();
				sound = control_soundwave47_tag.Component;
			}
		}
		if (sound)
			sound->Play();

		control_target1_tag.Component->MessageField = 0;
		control_target1_tag.Component->Message(50, 0.0);
		control_target2_tag.Component->MessageField = 0;
		control_target2_tag.Component->Message(50, 0.0);
		control_target3_tag.Component->MessageField = 0;
		control_target3_tag.Component->Message(50, 0.0);
		TableG->AddScore(caller->get_scoring(1));
	}
}

void control::MedalLightGroupControl(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 0:
		caller->Message(0, 0.0);
		break;
	case 41:
		caller->Message(41, 2.0);
		caller->Message(43, 30.0);
		break;
	case 61:
		caller->Message(33, 0.0);
		if (caller->Message(37, 0.0))
			caller->Message(43, 30.0);
		break;
	default: break;
	}
}

void control::MultiplierLightGroupControl(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 0:
		caller->Message(0, 0.0);
		break;
	case 41:
		caller->Message(41, 2.0);
		caller->Message(43, 30.0);
		break;
	case 61:
		if (TableG->ScoreMultiplier)
			TableG->ScoreMultiplier = TableG->ScoreMultiplier - 1;
		caller->Message(33, 0.0);
		if (caller->Message(37, 0.0))
			caller->Message(43, 30.0);
		break;
	case 64:
		TableG->ScoreMultiplier = 4;
		caller->Message(19, 0.0);
		caller->Message(43, 30.0);
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(59, 0), 2.0);
		break;
	case 65:
		TableG->ScoreMultiplier = 0;
		caller->Message(20, 0.0);
		caller->Message(43, -1.0);
		break;
	default:
		break;
	}
}

void control::FuelSpotTargetControl(int code, TPinballComponent* caller)
{
	TPinballComponent* liteComp;

	if (code == 63)
	{
		if (control_target10_tag.Component == caller)
		{
			liteComp = control_lite70_tag.Component;
		}
		else
		{
			liteComp = control_lite71_tag.Component;
			if (control_target11_tag.Component != caller)
				liteComp = control_lite72_tag.Component;
		}
		liteComp->Message(15, 2.0);
		TableG->AddScore(caller->get_scoring(0));
		if (control_top_circle_tgt_lights_tag.Component->Message(37, 0.0) == 3)
		{
			control_top_circle_tgt_lights_tag.Component->Message(16, 2.0);
			control_fuel_bargraph_tag.Component->Message(45, 11.0);
			control_soundwave25_tag.Component->Play();
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0);
		}
		else
		{
			control_soundwave49D_tag.Component->Play();
		}
	}
}

void control::MissionSpotTargetControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		TPinballComponent* lite;
		if (control_target13_tag.Component == caller)
		{
			control_lite101_tag.Component->MessageField |= 1u;
			lite = control_lite101_tag.Component;
		}
		else if (control_target14_tag.Component == caller)
		{
			control_lite101_tag.Component->MessageField |= 2u;
			lite = control_lite102_tag.Component;
		}
		else
		{
			control_lite101_tag.Component->MessageField |= 4u;
			lite = control_lite103_tag.Component;
		}
		lite->Message(15, 2.0);

		TSound* sound;
		if (!light_on(&control_lite198_tag) || control_lite198_tag.Component->FlasherActive)
		{
			sound = control_soundwave52_tag.Component;
		}
		else
			sound = control_soundwave49D_tag.Component;
		sound->Play();
		TableG->AddScore(caller->get_scoring(0));
		if (control_ramp_tgt_lights_tag.Component->Message(37, 0.0) == 3)
			control_ramp_tgt_lights_tag.Component->Message(16, 2.0);
	}
}

void control::LeftHazardSpotTargetControl(int code, TPinballComponent* caller)
{
	TPinballComponent* lite;

	if (code == 63)
	{
		if (control_target16_tag.Component == caller)
		{
			control_lite104_tag.Component->MessageField |= 1u;
			lite = control_lite104_tag.Component;
		}
		else if (control_target17_tag.Component == caller)
		{
			control_lite104_tag.Component->MessageField |= 2u;
			lite = control_lite105_tag.Component;
		}
		else
		{
			control_lite104_tag.Component->MessageField |= 4u;
			lite = control_lite106_tag.Component;
		}
		lite->Message(15, 2.0);
		TableG->AddScore(caller->get_scoring(0));
		if (control_lchute_tgt_lights_tag.Component->Message(37, 0.0) == 3)
		{
			control_soundwave14_1_tag.Component->Play();
			control_gate1_tag.Component->Message(53, 0.0);
			control_lchute_tgt_lights_tag.Component->Message(16, 2.0);
		}
		else
		{
			control_soundwave49D_tag.Component->Play();
		}
	}
}

void control::RightHazardSpotTargetControl(int code, TPinballComponent* caller)
{
	TPinballComponent* light;

	if (code == 63)
	{
		if (control_target19_tag.Component == caller)
		{
			control_lite107_tag.Component->MessageField |= 1u;
			light = control_lite107_tag.Component;
		}
		else if (control_target20_tag.Component == caller)
		{
			control_lite107_tag.Component->MessageField |= 2u;
			light = control_lite108_tag.Component;
		}
		else
		{
			control_lite107_tag.Component->MessageField |= 4u;
			light = control_lite109_tag.Component;
		}
		light->Message(15, 2.0);
		TableG->AddScore(caller->get_scoring(0));
		if (control_bpr_solotgt_lights_tag.Component->Message(37, 0.0) == 3)
		{
			control_soundwave14_1_tag.Component->Play();
			control_gate2_tag.Component->Message(53, 0.0);
			control_bpr_solotgt_lights_tag.Component->Message(16, 2.0);
		}
		else
		{
			control_soundwave49D_tag.Component->Play();
		}
	}
}

void control::WormHoleDestinationControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (!light_on(&control_lite110_tag))
		{
			control_lite110_tag.Component->Message(15, 3.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(93, 0), 2.0);
		}
		TableG->AddScore(caller->get_scoring(0));
		AdvanceWormHoleDestination(1);
	}
}

void control::BlackHoleKickoutControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		int addedScore = TableG->AddScore(caller->get_scoring(0));
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(80, 0), addedScore);
		control_info_text_box_tag.Component->Display(Buffer, 2.0);
		caller->Message(55, -1.0);
	}
}

void control::FlagControl(int code, TPinballComponent* caller)
{
	if (code == 62)
	{
		AdvanceWormHoleDestination(0);
	}
	else if (code == 63)
	{
		int score = caller->get_scoring(light_on(&control_lite20_tag));
		TableG->AddScore(score);
	}
}

void control::GravityWellKickoutControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	switch (code)
	{
	case 63:
		{
			auto addedScore = TableG->AddScore(caller->get_scoring(0));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(81, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			control_lite62_tag.Component->Message(20, 0.0);
			caller->ActiveFlag = 0;
			auto duration = control_soundwave7_tag.Component->Play();
			caller->Message(55, duration);
			break;
		}
	case 64:
		{
			auto score = reinterpret_cast<size_t>(caller);
			if (score)
			{
				snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(82, 0), score);
			}
			else
			{
				snprintf(Buffer, sizeof Buffer, "%s", pinball::get_rc_string(45, 0));
			}
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			control_lite62_tag.Component->Message(4, 0.0);
			control_kickout1_tag.Component->ActiveFlag = 1;
			break;
		}
	case 1024:
		control_kickout1_tag.Component->ActiveFlag = 0;
		break;
	}
}

void control::SkillShotGate1Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite200_tag.Component->Message(9, 5.0);
		if (light_on(&control_lite67_tag))
		{
			control_skill_shot_lights_tag.Component->Message(34, 0.0);
			control_skill_shot_lights_tag.Component->Message(20, 0.0);
			control_lite67_tag.Component->Message(19, 0.0);
			control_lite54_tag.Component->Message(7, 5.0);
			control_lite25_tag.Component->Message(7, 5.0);
			control_fuel_bargraph_tag.Component->Message(45, 11.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate2Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite68_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate3Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite69_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate4Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite131_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate5Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite132_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate6Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite133_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::ShootAgainLightControl(int code, TPinballComponent* caller)
{
	if (code == 60)
	{
		if (caller->MessageField)
		{
			caller->MessageField = 0;
		}
		else
		{
			caller->Message(16, 5.0);
			caller->MessageField = 1;
		}
	}
}

void control::EscapeChuteSinkControl(int code, TPinballComponent* caller)
{
	if (code == 63)
		caller->Message(56, static_cast<TSink*>(caller)->TimerTime);
}

void control::MissionControl(int code, TPinballComponent* caller)
{
	if (!control_lite198_tag.Component)
		return;

	int lite198Msg = control_lite198_tag.Component->MessageField;
	switch (code)
	{
	case 47:
		if (control_fuel_bargraph_tag.Component == caller && lite198Msg > 1)
		{
			control_l_trek_lights_tag.Component->Message(34, 0.0);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(34, 0.0);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(109, 0), 4.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
		}
		break;
	case 60:
		if (control_fuel_bargraph_tag.Component == caller && lite198Msg)
		{
			if (control_fuel_bargraph_tag.Component->Message(37, 0.0) == 1)
			{
				control_mission_text_box_tag.Component->Display(pinball::get_rc_string(116, 0), 4.0);
			}
			break;
		}
		if (control_mission_text_box_tag.Component == caller)
			code = 67;
		break;
	case 1009:
		code = 67;
		break;
	default:
		break;
	}

	switch (lite198Msg)
	{
	case 0:
		WaitingDeploymentController(code, caller);
		break;
	case 1:
		SelectMissionController(code, caller);
		break;
	case 2:
		PracticeMissionController(code, caller);
		break;
	case 3:
		LaunchTrainingController(code, caller);
		break;
	case 4:
		ReentryTrainingController(code, caller);
		break;
	case 5:
		ScienceMissionController(code, caller);
		break;
	case 6:
		StrayCometController(code, caller);
		break;
	case 7:
		BlackHoleThreatController(code, caller);
		break;
	case 8:
		SpaceRadiationController(code, caller);
		break;
	case 9:
		BugHuntController(code, caller);
		break;
	case 10:
		AlienMenaceController(code, caller);
		break;
	case 11:
		RescueMissionController(code, caller);
		break;
	case 12:
		SatelliteController(code, caller);
		break;
	case 13:
		ReconnaissanceController(code, caller);
		break;
	case 14:
		DoomsdayMachineController(code, caller);
		break;
	case 15:
		CosmicPlagueController(code, caller);
		break;
	case 16:
		SecretMissionYellowController(code, caller);
		break;
	case 17:
		TimeWarpController(code, caller);
		break;
	case 18:
		MaelstromController(code, caller);
		break;
	case 20:
		AlienMenacePartTwoController(code, caller);
		break;
	case 21:
		CosmicPlaguePartTwoController(code, caller);
		break;
	case 22:
		SecretMissionRedController(code, caller);
		break;
	case 23:
		SecretMissionGreenController(code, caller);
		break;
	case 24:
		TimeWarpPartTwoController(code, caller);
		break;
	case 25:
		MaelstromPartTwoController(code, caller);
		break;
	case 26:
		MaelstromPartThreeController(code, caller);
		break;
	case 27:
		MaelstromPartFourController(code, caller);
		break;
	case 28:
		MaelstromPartFiveController(code, caller);
		break;
	case 29:
		MaelstromPartSixController(code, caller);
		break;
	case 30:
		MaelstromPartSevenController(code, caller);
		break;
	case 31:
		MaelstromPartEightController(code, caller);
		break;
	case 32:
		GameoverController(code, caller);
		break;
	default:
		UnselectMissionController(code, caller);
		break;
	}
}

void control::HyperspaceKickOutControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
		return;

	auto activeCount = control_hyper_lights_tag.Component->Message(37, 0.0);
	HyperspaceLightGroupControl(41, control_hyper_lights_tag.Component);
	switch (activeCount)
	{
	case 0:
		{
			auto addedScore = TableG->AddScore(caller->get_scoring(0));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(12, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			break;
		}
	case 1:
		{
			auto addedScore = SpecialAddScore(TableG->ScoreSpecial3);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(14, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			TableG->ScoreSpecial3 = 20000;
			break;
		}
	case 2:
		{
			DrainBallBlockerControl(52, control_block1_tag.Component);
			auto addedScore = TableG->AddScore(caller->get_scoring(2));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(2, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			break;
		}
	case 3:
		{
			ExtraBallLightControl(19, nullptr);
			auto addedScore = TableG->AddScore(caller->get_scoring(3));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(8, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0);
			break;
		}
	case 4:
		{
			control_hyper_lights_tag.Component->Message(0, 0.0);
			size_t addedScore = TableG->AddScore(caller->get_scoring(4));
			GravityWellKickoutControl(64, reinterpret_cast<TPinballComponent*>(addedScore));
			break;
		}
	default:
		break;
	}

	int someFlag = 0;
	if (light_on(&control_lite25_tag))
	{
		someFlag = 1;
		auto addedScore = SpecialAddScore(TableG->ScoreSpecial1);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(10, 0), addedScore);
		control_info_text_box_tag.Component->Display(Buffer, 2.0);
	}
	if (light_on(&control_lite26_tag))
		someFlag |= 2u;
	if (light_on(&control_lite130_tag))
	{
		someFlag |= 4u;
		control_lite130_tag.Component->Message(20, 0.0);
		MultiplierLightGroupControl(64, control_top_target_lights_tag.Component);
		control_bumber_target_lights_tag.Component->Message(19, 0.0);
		table_set_jackpot();
		table_set_bonus();
		table_set_flag_lights();
		table_set_bonus_hold();
		control_lite27_tag.Component->Message(19, 0.0);
		control_lite28_tag.Component->Message(19, 0.0);
		ExtraBallLightControl(19, nullptr);
		DrainBallBlockerControl(52, control_block1_tag.Component);

		if (TableG->MultiballFlag)
		{
			table_set_multiball();
		}
		if (TableG->ScoreSpecial3 < 100000)
			TableG->ScoreSpecial3 = 100000;
		if (TableG->ScoreSpecial2 < 100000)
			TableG->ScoreSpecial2 = 100000;
		GravityWellKickoutControl(64, nullptr);
	}

	TSound* sound;
	if (someFlag)
	{
		if (someFlag == 1)
		{
			sound = control_soundwave21_tag.Component;
		}
		else
		{
			if (someFlag < 1 || someFlag > 3)
			{
				auto duration = control_soundwave41_tag.Component->Play();
				control_soundwave36_1_tag.Component->Play();
				control_soundwave50_2_tag.Component->Play();
				control_lite25_tag.Component->Message(7, 5.0);
				caller->Message(55, duration);
				return;
			}
			sound = control_soundwave40_tag.Component;
		}
	}
	else
	{
		switch (activeCount)
		{
		case 1:
			sound = control_soundwave36_2_tag.Component;
			break;
		case 2:
			sound = control_soundwave35_2_tag.Component;
			break;
		case 3:
			sound = control_soundwave38_tag.Component;
			break;
		case 4:
			sound = control_soundwave39_tag.Component;
			break;
		default:
			sound = control_soundwave35_1_tag.Component;
			break;
		}
	}
	auto duration = sound->Play();
	control_lite25_tag.Component->Message(7, 5.0);
	caller->Message(55, duration);
}

void control::PlungerControl(int code, TPinballComponent* caller)
{
	if (code == 1015)
	{
		MissionControl(67, nullptr);
	}
	else if (code == 1016)
	{
		table_unlimited_balls = false;
		if (!control_middle_circle_tag.Component->Message(37, 0.0))
			control_middle_circle_tag.Component->Message(32, 0.0);
		if (!light_on(&control_lite200_tag))
		{
			control_skill_shot_lights_tag.Component->Message(20, 0.0);
			control_lite67_tag.Component->Message(19, 0.0);
			control_skill_shot_lights_tag.Component->Message(26, 0.25f);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_l_trek_lights_tag.Component->Message(32, 0.2f);
			control_l_trek_lights_tag.Component->Message(26, 0.2f);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(32, 0.2f);
			control_r_trek_lights_tag.Component->Message(26, 0.2f);
			TableG->ScoreSpecial1 = 25000;
			MultiplierLightGroupControl(65, control_top_target_lights_tag.Component);
			control_fuel_bargraph_tag.Component->Message(19, 0.0);
			control_lite200_tag.Component->Message(19, 0.0);
			control_gate1_tag.Component->Message(53, 0.0);
			control_gate2_tag.Component->Message(53, 0.0);
		}
		control_lite200_tag.Component->MessageField = 0;
	}
}

void control::MedalTargetControl(int code, TPinballComponent* caller)
{
	if (code == 63 && !caller->MessageField)
	{
		caller->MessageField = 1;
		if (control_target6_tag.Component->MessageField
			+ control_target5_tag.Component->MessageField
			+ control_target4_tag.Component->MessageField == 3)
		{
			MedalLightGroupControl(41, control_bumber_target_lights_tag.Component);
			int activeCount = control_bumber_target_lights_tag.Component->Message(37, 0.0) - 1;
			char* text;
			switch (activeCount)
			{
			case 0:
				TableG->AddScore(caller->get_scoring(1));
				text = pinball::get_rc_string(53, 0);
				break;
			case 1:
				TableG->AddScore(caller->get_scoring(2));
				text = pinball::get_rc_string(54, 0);
				break;
			default:
				table_add_extra_ball(4.0);
				text = pinball::get_rc_string(55, 0);
				break;
			}
			control_info_text_box_tag.Component->Display(text, 2.0);
			control_target6_tag.Component->MessageField = 0;
			control_target6_tag.Component->Message(50, 0.0);
			control_target5_tag.Component->MessageField = 0;
			control_target5_tag.Component->Message(50, 0.0);
			control_target4_tag.Component->MessageField = 0;
			control_target4_tag.Component->Message(50, 0.0);
			return;
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::MultiplierTargetControl(int code, TPinballComponent* caller)
{
	if (code == 63 && !caller->MessageField)
	{
		caller->MessageField = 1;
		if (control_target9_tag.Component->MessageField
			+ control_target8_tag.Component->MessageField
			+ control_target7_tag.Component->MessageField == 3)
		{
			TableG->AddScore(caller->get_scoring(1));
			MultiplierLightGroupControl(41, control_top_target_lights_tag.Component);
			int activeCount = control_top_target_lights_tag.Component->Message(37, 0.0);
			char* text;
			switch (activeCount)
			{
			case 1:
				TableG->ScoreMultiplier = 1;
				text = pinball::get_rc_string(56, 0);
				break;
			case 2:
				TableG->ScoreMultiplier = 2;
				text = pinball::get_rc_string(57, 0);
				break;
			case 3:
				TableG->ScoreMultiplier = 3;
				text = pinball::get_rc_string(58, 0);
				break;
			default:
				TableG->ScoreMultiplier = 4;
				text = pinball::get_rc_string(59, 0);
				break;
			}

			control_info_text_box_tag.Component->Display(text, 2.0);
			control_target9_tag.Component->MessageField = 0;
			control_target9_tag.Component->Message(50, 0.0);
			control_target8_tag.Component->MessageField = 0;
			control_target8_tag.Component->Message(50, 0.0);
			control_target7_tag.Component->MessageField = 0;
			control_target7_tag.Component->Message(50, 0.0);
		}
		else
		{
			TableG->AddScore(caller->get_scoring(0));
		}
	}
}

void control::BallDrainControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 60)
	{
		if (control_lite199_tag.Component->MessageField)
		{
			TableG->Message(1022, 0.0);
			if (pb::chk_highscore())
			{
				control_soundwave3_tag.Component->Play();
				TableG->LightGroup->Message(16, 3.0);
				char* v11 = pinball::get_rc_string(177, 0);
				control_mission_text_box_tag.Component->Display(v11, -1.0);
			}
		}
		else
		{
			control_plunger_tag.Component->Message(1016, 0.0);
		}
	}
	else if (code == 63)
	{
		if (table_unlimited_balls)
		{
			control_drain_tag.Component->Message(1024, 0.0);
			control_sink3_tag.Component->Message(56, 0.0);
		}
		else
		{
			if (TableG->TiltLockFlag)
			{
				control_lite200_tag.Component->Message(20, 0.0);
				control_lite199_tag.Component->Message(20, 0.0);
			}
			if (light_on(&control_lite200_tag))
			{
				control_soundwave27_tag.Component->Play();
				control_lite200_tag.Component->Message(19, 0.0);
				control_info_text_box_tag.Component->Display(pinball::get_rc_string(96, 0), -1.0);
				control_soundwave59_tag.Component->Play();
			}
			else if (light_on(&control_lite199_tag))
			{
				control_soundwave27_tag.Component->Play();
				control_lite199_tag.Component->Message(20, 0.0);
				control_lite200_tag.Component->Message(19, 0.0);
				control_info_text_box_tag.Component->Display(pinball::get_rc_string(95, 0), 2.0);
				control_soundwave59_tag.Component->Play();
				--TableG->UnknownP78;
			}
			else if (TableG->UnknownP75)
			{
				control_soundwave27_tag.Component->Play();
				--TableG->UnknownP75;
			}
			else
			{
				if (!TableG->TiltLockFlag)
				{
					int time = SpecialAddScore(TableG->ScoreSpecial2);
					snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(94, 0), time);
					control_info_text_box_tag.Component->Display(Buffer, 2.0);
				}
				if (TableG->ExtraBalls)
				{
					TableG->ExtraBalls--;

					char* shootAgainText;
					control_soundwave59_tag.Component->Play();
					switch (TableG->CurrentPlayer)
					{
					case 0:
						shootAgainText = pinball::get_rc_string(97, 0);
						break;
					case 1:
						shootAgainText = pinball::get_rc_string(98, 0);
						break;
					case 2:
						shootAgainText = pinball::get_rc_string(99, 0);
						break;
					default:
					case 3:
						shootAgainText = pinball::get_rc_string(100, 0);
						break;
					}
					control_info_text_box_tag.Component->Display(shootAgainText, -1.0);
				}
				else
				{
					TableG->ChangeBallCount(TableG->BallCount - 1);
					if (TableG->CurrentPlayer + 1 != TableG->PlayerCount || TableG->BallCount)
					{
						TableG->Message(1021, 0.0);
						control_lite199_tag.Component->MessageField = 0;
					}
					else
					{
						control_lite199_tag.Component->MessageField = 1;
					}
					control_soundwave27_tag.Component->Play();
				}
				control_bmpr_inc_lights_tag.Component->Message(20, 0.0);
				control_ramp_bmpr_inc_lights_tag.Component->Message(20, 0.0);
				control_lite30_tag.Component->Message(20, 0.0);
				control_lite29_tag.Component->Message(20, 0.0);
				control_lite1_tag.Component->Message(20, 0.0);
				control_lite54_tag.Component->Message(20, 0.0);
				control_lite55_tag.Component->Message(20, 0.0);
				control_lite56_tag.Component->Message(20, 0.0);
				control_lite17_tag.Component->Message(20, 0.0);
				control_lite18_tag.Component->Message(20, 0.0);
				control_lite27_tag.Component->Message(20, 0.0);
				control_lite28_tag.Component->Message(20, 0.0);
				control_lite16_tag.Component->Message(20, 0.0);
				control_lite20_tag.Component->Message(20, 0.0);
				control_hyper_lights_tag.Component->Message(20, 0.0);
				control_lite25_tag.Component->Message(20, 0.0);
				control_lite26_tag.Component->Message(20, 0.0);
				control_lite130_tag.Component->Message(20, 0.0);
				control_lite19_tag.Component->Message(20, 0.0);
				control_worm_hole_lights_tag.Component->Message(20, 0.0);
				control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
				control_l_trek_lights_tag.Component->Message(20, 0.0);
				control_r_trek_lights_tag.Component->Message(20, 0.0);
				control_lite60_tag.Component->Message(20, 0.0);
				control_lite59_tag.Component->Message(20, 0.0);
				control_lite61_tag.Component->Message(20, 0.0);
				control_bumber_target_lights_tag.Component->Message(20, 0.0);
				control_top_target_lights_tag.Component->Message(20, 0.0);
				control_top_circle_tgt_lights_tag.Component->Message(20, 0.0);
				control_ramp_tgt_lights_tag.Component->Message(20, 0.0);
				control_lchute_tgt_lights_tag.Component->Message(20, 0.0);
				control_bpr_solotgt_lights_tag.Component->Message(20, 0.0);
				control_lite110_tag.Component->Message(20, 0.0);
				control_skill_shot_lights_tag.Component->Message(20, 0.0);
				control_lite77_tag.Component->Message(20, 0.0);
				control_lite198_tag.Component->Message(20, 0.0);
				control_lite196_tag.Component->Message(20, 0.0);
				control_lite195_tag.Component->Message(20, 0.0);
				control_fuel_bargraph_tag.Component->Message(20, 0.0);
				control_fuel_bargraph_tag.Component->Message(1024, 0.0);
				GravityWellKickoutControl(1024, nullptr);
				control_lite62_tag.Component->Message(20, 0.0);
				control_lite4_tag.Component->MessageField = 0;
				control_lite101_tag.Component->MessageField = 0;
				control_lite102_tag.Component->MessageField = 0;
				control_lite103_tag.Component->MessageField = 0;
				control_ramp_tgt_lights_tag.Component->MessageField = 0;
				control_outer_circle_tag.Component->Message(34, 0.0);
				control_middle_circle_tag.Component->Message(34, 0.0);
				control_attack_bump_tag.Component->Message(1024, 0.0);
				control_launch_bump_tag.Component->Message(1024, 0.0);
				control_gate1_tag.Component->Message(1024, 0.0);
				control_gate2_tag.Component->Message(1024, 0.0);
				control_block1_tag.Component->Message(1024, 0.0);
				control_target1_tag.Component->Message(1024, 0.0);
				control_target2_tag.Component->Message(1024, 0.0);
				control_target3_tag.Component->Message(1024, 0.0);
				control_target6_tag.Component->Message(1024, 0.0);
				control_target5_tag.Component->Message(1024, 0.0);
				control_target4_tag.Component->Message(1024, 0.0);
				control_target9_tag.Component->Message(1024, 0.0);
				control_target8_tag.Component->Message(1024, 0.0);
				control_target7_tag.Component->Message(1024, 0.0);
				if (control_lite199_tag.Component->MessageField)
					control_lite198_tag.Component->MessageField = 32;
				else
					control_lite198_tag.Component->MessageField = 0;
				MissionControl(66, nullptr);
				TableG->Message(1012, 0.0);
				if (light_on(&control_lite58_tag))
					control_lite58_tag.Component->Message(20, 0.0);
				else
					TableG->ScoreSpecial2 = 25000;
			}
		}
	}
}


void control::table_control_handler(int code)
{
	if (code == 1011)
	{
		table_unlimited_balls = false;
		control_lite77_tag.Component->Message(7, 0.0);
	}
}


void control::AlienMenaceController(int code, TPinballComponent* caller)
{
	if (code != 11)
	{
		if (code == 66)
		{
			control_attack_bump_tag.Component->Message(11, 0.0);
			TPinballComponent* lTrekLight = control_l_trek_lights_tag.Component;
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			lTrekLight->Message(32, 0.2f);
			lTrekLight->Message(26, 0.2f);
			TPinballComponent* rTrekLight = control_r_trek_lights_tag.Component;
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			rTrekLight->Message(32, 0.2f);
			rTrekLight->Message(26, 0.2f);
			control_lite307_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(175, 0), -1.0);
		return;
	}
	if (control_bump1_tag.Component == caller)
	{
		if (control_bump1_tag.Component->BmpIndex)
		{
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 20;
			MissionControl(66, nullptr);
		}
	}
}

void control::AlienMenacePartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 8;
			control_l_trek_lights_tag.Component->Message(34, 0.0);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(34, 0.0);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite311_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(107, 0),
		          control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_bump1_tag.Component == caller
		|| control_bump2_tag.Component == caller
		|| control_bump3_tag.Component == caller
		|| control_bump4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite311_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(130, 0), 4.0);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(7))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::BlackHoleThreatController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 11)
	{
		if (control_bump5_tag.Component == caller)
			MissionControl(67, caller);
	}
	else if (code == 63)
	{
		if (control_kickout3_tag.Component == caller
			&& control_bump5_tag.Component->BmpIndex)
		{
			if (light_on(&control_lite316_tag))
				control_lite316_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite314_tag))
				control_lite314_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(124, 0), 4.0);
			int addedScore = SpecialAddScore(1000000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(8))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
	else
	{
		if (code == 66)
		{
			control_launch_bump_tag.Component->Message(11, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		if (control_bump5_tag.Component->BmpIndex)
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(123, 0), -1.0);
			if (light_on(&control_lite316_tag))
				control_lite316_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite314_tag))
			{
				control_lite314_tag.Component->Message(7, 0.0);
			}
		}
		else
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(122, 0), -1.0);
			if (light_on(&control_lite314_tag))
				control_lite314_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite316_tag))
			{
				control_lite316_tag.Component->Message(7, 0.0);
			}
		}
	}
}

void control::BugHuntController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 15;
			control_target1_tag.Component->MessageField = 0;
			control_target1_tag.Component->Message(50, 0.0);
			control_target2_tag.Component->MessageField = 0;
			control_target2_tag.Component->Message(50, 0.0);
			control_target3_tag.Component->MessageField = 0;
			control_target3_tag.Component->Message(50, 0.0);
			control_target6_tag.Component->MessageField = 0;
			control_target6_tag.Component->Message(50, 0.0);
			control_target5_tag.Component->MessageField = 0;
			control_target5_tag.Component->Message(50, 0.0);
			control_target4_tag.Component->MessageField = 0;
			control_target4_tag.Component->Message(50, 0.0);
			control_target9_tag.Component->MessageField = 0;
			control_target9_tag.Component->Message(50, 0.0);
			control_target8_tag.Component->MessageField = 0;
			control_target8_tag.Component->Message(50, 0.0);
			control_target7_tag.Component->MessageField = 0;
			control_target7_tag.Component->Message(50, 0.0);
			control_top_circle_tgt_lights_tag.Component->Message(20, 0.0);
			control_ramp_tgt_lights_tag.Component->Message(20, 0.0);
			control_lchute_tgt_lights_tag.Component->Message(20, 0.0);
			control_bpr_solotgt_lights_tag.Component->Message(20, 0.0);
			control_lite306_tag.Component->Message(7, 0.0);
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite310_tag.Component->Message(7, 0.0);
			control_lite313_tag.Component->Message(7, 0.0);
			control_lite319_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(125, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_target1_tag.Component == caller
		|| control_target2_tag.Component == caller
		|| control_target3_tag.Component == caller
		|| control_target6_tag.Component == caller
		|| control_target5_tag.Component == caller
		|| control_target4_tag.Component == caller
		|| control_target9_tag.Component == caller
		|| control_target8_tag.Component == caller
		|| control_target7_tag.Component == caller
		|| control_target10_tag.Component == caller
		|| control_target11_tag.Component == caller
		|| control_target12_tag.Component == caller
		|| control_target13_tag.Component == caller
		|| control_target14_tag.Component == caller
		|| control_target15_tag.Component == caller
		|| control_target16_tag.Component == caller
		|| control_target17_tag.Component == caller
		|| control_target18_tag.Component == caller
		|| control_target19_tag.Component == caller
		|| control_target20_tag.Component == caller
		|| control_target21_tag.Component == caller
		|| control_target22_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite306_tag.Component->Message(20, 0.0);
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite310_tag.Component->Message(20, 0.0);
			control_lite313_tag.Component->Message(20, 0.0);
			control_lite319_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(126, 0), 4.0);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(7))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::CosmicPlagueController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 75;
			control_lite20_tag.Component->Message(19, 0.0);
			control_lite19_tag.Component->Message(19, 0.0);
			control_lite305_tag.Component->Message(7, 0.0);
			control_lite312_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(139, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_flag1_tag.Component == caller || control_flag2_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite305_tag.Component->Message(20, 0.0);
			control_lite312_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 21;
			MissionControl(66, nullptr);
			control_lite20_tag.Component->Message(20, 0.0);
			control_lite19_tag.Component->Message(20, 0.0);
		}
	}
}

void control::CosmicPlaguePartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite310_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(140, 0), -1.0);
		return;
	}
	if (control_roll9_tag.Component == caller)
	{
		control_lite310_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 1;
		MissionControl(66, nullptr);
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(141, 0), 4.0);
		int addedScore = SpecialAddScore(1750000);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
		if (!AddRankProgress(11))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0);
			control_soundwave9_tag.Component->Play();
		}
	}
}

void control::DoomsdayMachineController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite301_tag.Component->Message(7, 0.0);
			control_lite320_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(137, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_roll4_tag.Component == caller || control_roll8_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite301_tag.Component->Message(20, 0.0);
			control_lite320_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(138, 0), 4.0);
			int addedScore = SpecialAddScore(1250000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::GameoverController(int code, TPinballComponent* caller)
{
	char Buffer[128];

	if (code == 66)
	{
		control_goal_lights_tag.Component->Message(20, 0.0);
		pb::mode_change(2);
		control_flip1_tag.Component->Message(1022, 0.0);
		control_flip2_tag.Component->Message(1022, 0.0);
		control_mission_text_box_tag.Component->MessageField = 0;
		return;
	}
	if (code != 67)
		return;

	int missionMsg = control_mission_text_box_tag.Component->MessageField;
	if (missionMsg & 0x100)
	{
		int playerId = missionMsg % 4;
		int playerScore = TableG->PlayerScores[playerId].ScoreStruct->Score;
		auto nextPlayerId = playerId + 1;
		if (playerScore >= 0)
		{
			const char* playerNScoreText = nullptr;
			switch (nextPlayerId)
			{
			case 1:
				playerNScoreText = pinball::get_rc_string(180, 0);
				break;
			case 2:
				playerNScoreText = pinball::get_rc_string(181, 0);
				break;
			case 3:
				playerNScoreText = pinball::get_rc_string(182, 0);
				break;
			case 4:
				playerNScoreText = pinball::get_rc_string(183, 0);
				break;
			default:
				break;
			}
			if (playerNScoreText != nullptr)
			{
				snprintf(Buffer, sizeof Buffer, playerNScoreText, playerScore);
				control_mission_text_box_tag.Component->Display(Buffer, 3.0);
				int msgField = nextPlayerId == TableG->PlayerCount ? 0x200 : nextPlayerId | 0x100;
				control_mission_text_box_tag.Component->MessageField = msgField;
				return;
			}
		}
		control_mission_text_box_tag.Component->MessageField = 0x200;
	}

	if (missionMsg & 0x200)
	{
		int highscoreId = missionMsg % 5;
		int highScore = pb::highscore_table[highscoreId].Score;
		auto nextHidhscoreId = highscoreId + 1;
		if (highScore > 0)
		{
			const char* highScoreNText = nullptr;
			switch (nextHidhscoreId)
			{
			case 1:
				highScoreNText = pinball::get_rc_string(184, 0);
				break;
			case 2:
				highScoreNText = pinball::get_rc_string(185, 0);
				break;
			case 3:
				highScoreNText = pinball::get_rc_string(186, 0);
				break;
			case 4:
				highScoreNText = pinball::get_rc_string(187, 0);
				break;
			case 5:
				highScoreNText = pinball::get_rc_string(188, 0);
				break;
			default:
				break;
			}
			if (highScoreNText != nullptr)
			{
				snprintf(Buffer, sizeof Buffer, highScoreNText, highScore);
				control_mission_text_box_tag.Component->Display(Buffer, 3.0);
				int msgField = nextHidhscoreId == 5 ? 0 : nextHidhscoreId | 0x200;
				control_mission_text_box_tag.Component->MessageField = msgField;
				return;
			}
		}
	}

	control_mission_text_box_tag.Component->MessageField = 0x100;
	control_mission_text_box_tag.Component->Display(pinball::get_rc_string(172, 0), 10.0);
}

void control::LaunchTrainingController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite317_tag.Component->Message(7, 0.0);
			control_lite56_tag.Component->MessageField = 3;
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(110, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_ramp_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite317_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(111, 0), 4.0);
			int addedScore = SpecialAddScore(500000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(6))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::MaelstromController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite303_tag.Component->Message(7, 0.0);
			control_lite309_tag.Component->Message(7, 0.0);
			control_lite315_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(148, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_target1_tag.Component == caller
		|| control_target2_tag.Component == caller
		|| control_target3_tag.Component == caller
		|| control_target6_tag.Component == caller
		|| control_target5_tag.Component == caller
		|| control_target4_tag.Component == caller
		|| control_target9_tag.Component == caller
		|| control_target8_tag.Component == caller
		|| control_target7_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite303_tag.Component->Message(20, 0.0);
			control_lite309_tag.Component->Message(20, 0.0);
			control_lite315_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 25;
			MissionControl(66, nullptr);
		}
	}
}

void control::MaelstromPartEightController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite130_tag.Component->Message(19, 0.0);
			control_lite304_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(155, 0), -1.0);
		return;
	}
	if (control_kickout2_tag.Component == caller)
	{
		control_lite304_tag.Component->Message(20, 0.0);
		control_lite130_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 1;
		MissionControl(66, nullptr);
		int addedScore = SpecialAddScore(5000000);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(48, 0), 4.0);
		if (!AddRankProgress(18))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0);
			control_soundwave9_tag.Component->Play();
		}
	}
}

void control::MaelstromPartFiveController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite317_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(152, 0), -1.0);
		return;
	}
	if (control_ramp_tag.Component == caller)
	{
		control_lite317_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 29;
		MissionControl(66, nullptr);
	}
}

void control::MaelstromPartFourController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 0;
			control_lite318_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(151, 0), -1.0);
		return;
	}
	if (control_roll184_tag.Component == caller)
	{
		control_lite318_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 28;
		MissionControl(66, nullptr);
	}
}

void control::MaelstromPartSevenController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			AdvanceWormHoleDestination(1);
			control_sink1_tag.Component->Message(7, 0.0);
			control_sink2_tag.Component->Message(7, 0.0);
			control_sink3_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(154, 0), -1.0);
		return;
	}
	if (control_sink1_tag.Component == caller
		|| control_sink2_tag.Component == caller
		|| control_sink3_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 31;
		MissionControl(66, nullptr);
	}
}

void control::MaelstromPartSixController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite20_tag.Component->Message(19, 0.0);
			control_lite19_tag.Component->Message(19, 0.0);
			control_lite305_tag.Component->Message(7, 0.0);
			control_lite312_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(153, 0), -1.0);
		return;
	}
	if (control_flag1_tag.Component == caller || control_flag2_tag.Component == caller)
	{
		control_lite305_tag.Component->Message(20, 0.0);
		control_lite312_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 30;
		MissionControl(66, nullptr);
		control_lite20_tag.Component->Message(20, 0.0);
		control_lite19_tag.Component->Message(20, 0.0);
	}
}

void control::MaelstromPartThreeController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 5;
			control_lite301_tag.Component->Message(7, 0.0);
			control_lite302_tag.Component->Message(7, 0.0);
			control_lite307_tag.Component->Message(7, 0.0);
			control_lite316_tag.Component->Message(7, 0.0);
			control_lite320_tag.Component->Message(7, 0.0);
			control_lite321_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(150, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_roll3_tag.Component == caller
		|| control_roll2_tag.Component == caller
		|| control_roll1_tag.Component == caller
		|| control_roll112_tag.Component == caller
		|| control_roll111_tag.Component == caller
		|| control_roll110_tag.Component == caller
		|| control_roll4_tag.Component == caller
		|| control_roll8_tag.Component == caller
		|| control_roll6_tag.Component == caller
		|| control_roll7_tag.Component == caller
		|| control_roll5_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite301_tag.Component->Message(20, 0.0);
			control_lite302_tag.Component->Message(20, 0.0);
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite316_tag.Component->Message(20, 0.0);
			control_lite320_tag.Component->Message(20, 0.0);
			control_lite321_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 27;
			MissionControl(66, nullptr);
		}
	}
}

void control::MaelstromPartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite306_tag.Component->Message(7, 0.0);
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite310_tag.Component->Message(7, 0.0);
			control_lite313_tag.Component->Message(7, 0.0);
			control_lite319_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(149, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_target10_tag.Component == caller
		|| control_target11_tag.Component == caller
		|| control_target12_tag.Component == caller
		|| control_target13_tag.Component == caller
		|| control_target14_tag.Component == caller
		|| control_target15_tag.Component == caller
		|| control_target16_tag.Component == caller
		|| control_target17_tag.Component == caller
		|| control_target18_tag.Component == caller
		|| control_target19_tag.Component == caller
		|| control_target20_tag.Component == caller
		|| control_target21_tag.Component == caller
		|| control_target22_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite306_tag.Component->Message(20, 0.0);
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite310_tag.Component->Message(20, 0.0);
			control_lite313_tag.Component->Message(20, 0.0);
			control_lite319_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 26;
			MissionControl(66, nullptr);
		}
	}
}

void control::PracticeMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite311_tag.Component->Message(7, 0.0);
			control_lite56_tag.Component->MessageField = 8;
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(107, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}

	if (control_bump1_tag.Component == caller
		|| control_bump2_tag.Component == caller
		|| control_bump3_tag.Component == caller
		|| control_bump4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite311_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(108, 0), 4.0);
			int addedScore = SpecialAddScore(500000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(6))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::ReconnaissanceController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 15;
			control_lite301_tag.Component->Message(7, 0.0);
			control_lite302_tag.Component->Message(7, 0.0);
			control_lite307_tag.Component->Message(7, 0.0);
			control_lite316_tag.Component->Message(7, 0.0);
			control_lite320_tag.Component->Message(7, 0.0);
			control_lite321_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(134, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_roll3_tag.Component == caller
		|| control_roll2_tag.Component == caller
		|| control_roll1_tag.Component == caller
		|| control_roll112_tag.Component == caller
		|| control_roll111_tag.Component == caller
		|| control_roll110_tag.Component == caller
		|| control_roll4_tag.Component == caller
		|| control_roll8_tag.Component == caller
		|| control_roll6_tag.Component == caller
		|| control_roll7_tag.Component == caller
		|| control_roll5_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, nullptr);
		}
		else
		{
			control_lite301_tag.Component->Message(20, 0.0);
			control_lite302_tag.Component->Message(20, 0.0);
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite316_tag.Component->Message(20, 0.0);
			control_lite320_tag.Component->Message(20, 0.0);
			control_lite321_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(136, 0), 4.0);
			int addedScore = SpecialAddScore(1250000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::ReentryTrainingController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			static_cast<TPinballComponent*>(control_l_trek_lights_tag.Component)->Message(32, 0.2f);
			static_cast<TPinballComponent*>(control_l_trek_lights_tag.Component)->Message(26, 0.2f);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			static_cast<TPinballComponent*>(control_r_trek_lights_tag.Component)->Message(32, 0.2f);
			static_cast<TPinballComponent*>(control_r_trek_lights_tag.Component)->Message(26, 0.2f);
			control_lite307_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(112, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_roll3_tag.Component == caller
		|| control_roll2_tag.Component == caller
		|| control_roll1_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(113, 0), 4.0);
			int addedScore = SpecialAddScore(500000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(6))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::RescueMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	switch (code)
	{
	case 63:
		{
			if (control_target1_tag.Component == caller
				|| control_target2_tag.Component == caller
				|| control_target3_tag.Component == caller)
			{
				MissionControl(67, caller);
				return;
			}
			if (control_kickout2_tag.Component != caller || !light_on(&control_lite20_tag))
				return;
			control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
			if (control_lite56_tag.Component->MessageField)
			{
				MissionControl(67, caller);
				return;
			}
			if (light_on(&control_lite303_tag))
				control_lite303_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite304_tag))
				control_lite304_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(129, 0), 4.0);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(7))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
			break;
		}
	case 66:
		control_lite20_tag.Component->Message(20, 0.0);
		control_lite19_tag.Component->Message(20, 0.0);
		control_lite56_tag.Component->MessageField = 1;
		break;
	case 67:
		if (light_on(&control_lite20_tag))
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(128, 0), -1.0);
			if (light_on(&control_lite303_tag))
				control_lite303_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite304_tag))
			{
				control_lite304_tag.Component->Message(7, 0.0);
			}
		}
		else
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(127, 0), -1.0);
			if (light_on(&control_lite304_tag))
				control_lite304_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite303_tag))
			{
				control_lite303_tag.Component->Message(7, 0.0);
			}
		}
		break;
	default:
		break;
	}
}

void control::SatelliteController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite308_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(132, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_bump4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(133, 0), 4.0);
			int addedScore = SpecialAddScore(1250000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::ScienceMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 9;
			control_target1_tag.Component->MessageField = 0;
			control_target1_tag.Component->Message(50, 0.0);
			control_target2_tag.Component->MessageField = 0;
			control_target2_tag.Component->Message(50, 0.0);
			control_target3_tag.Component->MessageField = 0;
			control_target3_tag.Component->Message(50, 0.0);
			control_target6_tag.Component->MessageField = 0;
			control_target6_tag.Component->Message(50, 0.0);
			control_target5_tag.Component->MessageField = 0;
			control_target5_tag.Component->Message(50, 0.0);
			control_target4_tag.Component->MessageField = 0;
			control_target4_tag.Component->Message(50, 0.0);
			control_target9_tag.Component->MessageField = 0;
			control_target9_tag.Component->Message(50, 0.0);
			control_target8_tag.Component->MessageField = 0;
			control_target8_tag.Component->Message(50, 0.0);
			control_target7_tag.Component->MessageField = 0;
			control_target7_tag.Component->Message(50, 0.0);
			control_lite303_tag.Component->Message(7, 0.0);
			control_lite309_tag.Component->Message(7, 0.0);
			control_lite315_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(114, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_target1_tag.Component == caller
		|| control_target2_tag.Component == caller
		|| control_target3_tag.Component == caller
		|| control_target6_tag.Component == caller
		|| control_target5_tag.Component == caller
		|| control_target4_tag.Component == caller
		|| control_target9_tag.Component == caller
		|| control_target8_tag.Component == caller
		|| control_target7_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite303_tag.Component->Message(20, 0.0);
			control_lite309_tag.Component->Message(20, 0.0);
			control_lite315_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(115, 0), 4.0);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::SecretMissionGreenController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite6_tag.Component->Message(19, 0.0);
			control_lite2_tag.Component->Message(11, 1.0);
			control_lite2_tag.Component->Message(19, 0.0);
			control_lite2_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		char* v2 = pinball::get_rc_string(144, 0);
		control_mission_text_box_tag.Component->Display(v2, -1.0);
		return;
	}
	if (control_sink2_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 1;
		MissionControl(66, nullptr);
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(145, 0), 4.0);
		int addedScore = SpecialAddScore(1500000);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
		if (!AddRankProgress(10))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0);
			control_soundwave9_tag.Component->Play();
		}
	}
}

void control::SecretMissionRedController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite5_tag.Component->Message(19, 0.0);
			control_lite4_tag.Component->Message(11, 2.0);
			control_lite4_tag.Component->Message(19, 0.0);
			control_lite4_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(143, 0), -1.0);
		return;
	}
	if (control_sink1_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 23;
		MissionControl(66, nullptr);
	}
}

void control::SecretMissionYellowController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_worm_hole_lights_tag.Component->Message(20, 0.0);
			control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
			control_bsink_arrow_lights_tag.Component->Message(23, 0.0);
			control_lite110_tag.Component->Message(20, 0.0);
			control_lite7_tag.Component->Message(19, 0.0);
			control_lite3_tag.Component->Message(11, 0.0);
			control_lite3_tag.Component->Message(19, 0.0);
			control_lite3_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(142, 0), -1.0);
		return;
	}
	if (control_sink3_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 22;
		MissionControl(66, nullptr);
	}
}

void control::SelectMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	switch (code)
	{
	case 45:
	case 47:
		if (control_fuel_bargraph_tag.Component != caller)
			return;
		MissionControl(67, caller);
		return;
	case 63:
		{
			int missionLevel = 0;
			if (control_target13_tag.Component == caller)
				missionLevel = 1;
			if (control_target14_tag.Component == caller)
				missionLevel = 2;
			if (control_target15_tag.Component == caller)
				missionLevel = 3;
			if (!missionLevel)
			{
				if (control_ramp_tag.Component == caller
					&& light_on(&control_lite56_tag)
					&& control_fuel_bargraph_tag.Component->Message(37, 0.0))
				{
					control_lite56_tag.Component->Message(20, 0.0);
					control_lite198_tag.Component->Message(19, 0.0);
					control_outer_circle_tag.Component->Message(26, -1.0);
					if (light_on(&control_lite317_tag))
						control_lite317_tag.Component->Message(20, 0.0);
					if (light_on(&control_lite318_tag))
						control_lite318_tag.Component->Message(20, 0.0);
					if (light_on(&control_lite319_tag))
						control_lite319_tag.Component->Message(20, 0.0);
					control_lite198_tag.Component->MessageField = control_lite56_tag.Component->MessageField;
					auto scoreId = control_lite56_tag.Component->MessageField - 2;
					MissionControl(66, nullptr);
					int addedScore = SpecialAddScore(mission_select_scores[scoreId]);
					snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(77, 0), addedScore);
					control_mission_text_box_tag.Component->Display(Buffer, 4.0);
				}
				return;
			}

			if (control_lite101_tag.Component->MessageField == 7)
			{
				control_lite101_tag.Component->MessageField = 0;
				missionLevel = 4;
			}

			int missionId;
			auto activeCount = control_middle_circle_tag.Component->Message(37, 0.0);
			switch (activeCount)
			{
			case 1:
				switch (missionLevel)
				{
				case 1:
					missionId = 3;
					break;
				case 2:
					missionId = 4;
					break;
				case 3:
					missionId = 2;
					break;
				default:
					missionId = 5;
					break;
				}
				break;
			case 2:
			case 3:
				switch (missionLevel)
				{
				case 1:
					missionId = 9;
					break;
				case 2:
					missionId = 11;
					break;
				case 3:
					missionId = 10;
					break;
				default:
					missionId = 16;
					break;
				}
				break;
			case 4:
			case 5:
				switch (missionLevel)
				{
				case 1:
					missionId = 6;
					break;
				case 2:
					missionId = 8;
					break;
				case 3:
					missionId = 7;
					break;
				default:
					missionId = 15;
					break;
				}
				break;
			case 6:
			case 7:
				switch (missionLevel)
				{
				case 1:
					missionId = 12;
					break;
				case 2:
					missionId = 13;
					break;
				case 3:
					missionId = 14;
					break;
				default:
					missionId = 17;
					break;
				}
				break;
			case 8:
			case 9:
				switch (missionLevel)
				{
				case 1:
					missionId = 15;
					break;
				case 2:
					missionId = 16;
					break;
				case 3:
					missionId = 17;
					break;
				default:
					missionId = 18;
					break;
				}
				break;
			default:
				return;
			}
			control_lite56_tag.Component->MessageField = missionId;
			control_lite56_tag.Component->Message(15, 2.0);
			control_lite198_tag.Component->Message(4, 0.0);
			MissionControl(67, caller);
			return;
		}
	case 66:
		control_lite198_tag.Component->Message(20, 0.0);
		control_outer_circle_tag.Component->Message(34, 0.0);
		control_ramp_tgt_lights_tag.Component->Message(20, 0.0);
		control_lite56_tag.Component->MessageField = 0;
		control_lite101_tag.Component->MessageField = 0;
		control_l_trek_lights_tag.Component->Message(34, 0.0);
		control_l_trek_lights_tag.Component->Message(20, 0.0);
		control_r_trek_lights_tag.Component->Message(34, 0.0);
		control_r_trek_lights_tag.Component->Message(20, 0.0);
		control_goal_lights_tag.Component->Message(20, 0.0);
		break;
	case 67:
		break;
	default:
		return;
	}

	if (control_fuel_bargraph_tag.Component->Message(37, 0.0))
	{
		if (light_on(&control_lite56_tag))
		{
			auto missionText = pinball::
				get_rc_string(MissionRcArray[control_lite56_tag.Component->MessageField - 2], 1);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(106, 0), missionText);
			control_mission_text_box_tag.Component->Display(Buffer, -1.0);
			if (light_on(&control_lite318_tag))
				control_lite318_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite319_tag))
				control_lite319_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite317_tag))
				control_lite317_tag.Component->Message(7, 0.0);
		}
		else
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(104, 0), -1.0);
			if (light_on(&control_lite317_tag))
				control_lite317_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite318_tag))
				control_lite318_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite319_tag))
			{
				control_lite319_tag.Component->Message(7, 0.0);
			}
		}
	}
	else
	{
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(105, 0), -1.0);
		if (light_on(&control_lite317_tag))
			control_lite317_tag.Component->Message(20, 0.0);
		if (light_on(&control_lite319_tag))
			control_lite319_tag.Component->Message(20, 0.0);
		if (!light_on(&control_lite318_tag))
		{
			control_lite318_tag.Component->Message(7, 0.0);
		}
	}
}

void control::SpaceRadiationController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		if (control_target16_tag.Component == caller
			|| control_target17_tag.Component == caller
			|| control_target18_tag.Component == caller)
		{
			if (control_lite104_tag.Component->MessageField == 7)
			{
				control_lite104_tag.Component->MessageField = 15;
				control_bsink_arrow_lights_tag.Component->Message(7, 0.0);
				control_lite313_tag.Component->Message(20, 0.0);
				MissionControl(67, caller);
				AdvanceWormHoleDestination(1);
			}
		}
		else if ((control_sink1_tag.Component == caller
				|| control_sink2_tag.Component == caller
				|| control_sink3_tag.Component == caller)
			&& control_lite104_tag.Component->MessageField == 15)
		{
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(121, 0), 4.0);
			int addedScore = SpecialAddScore(1000000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(8))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
	else
	{
		if (code == 66)
		{
			control_lchute_tgt_lights_tag.Component->Message(20, 0.0);
			control_lite104_tag.Component->MessageField = 0;
			control_lite313_tag.Component->Message(7, 0.0);
		}
		else if (code == 67)
		{
			char* text;
			if (control_lite104_tag.Component->MessageField == 15)
				text = pinball::get_rc_string(120, 0);
			else
				text = pinball::get_rc_string(176, 0);
			control_mission_text_box_tag.Component->Display(text, -1.0);
		}
	}
}

void control::StrayCometController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		if (control_target19_tag.Component == caller
			|| control_target20_tag.Component == caller
			|| control_target21_tag.Component == caller)
		{
			if (control_lite107_tag.Component->MessageField == 7)
			{
				control_lite306_tag.Component->Message(20, 0.0);
				control_lite304_tag.Component->Message(7, 0.0);
				control_lite107_tag.Component->MessageField = 15;
				MissionControl(67, caller);
			}
		}
		else if (control_kickout2_tag.Component == caller && control_lite107_tag.Component->MessageField == 15)
		{
			control_lite304_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(119, 0), 4.0);
			int addedScore = SpecialAddScore(1000000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(8))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
	else
	{
		if (code == 66)
		{
			control_bpr_solotgt_lights_tag.Component->Message(20, 0.0);
			control_lite107_tag.Component->MessageField = 0;
			control_lite306_tag.Component->Message(7, 0.0);
		}
		else if (code == 67)
		{
			char* text;
			if (control_lite107_tag.Component->MessageField == 15)
				text = pinball::get_rc_string(118, 0);
			else
				text = pinball::get_rc_string(117, 0);
			control_mission_text_box_tag.Component->Display(text, -1.0);
		}
	}
}

void control::TimeWarpController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 25;
			control_lite300_tag.Component->Message(7, 0.0);
			control_lite322_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(146, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0);
		return;
	}
	if (control_rebo1_tag.Component == caller
		|| control_rebo2_tag.Component == caller
		|| control_rebo3_tag.Component == caller
		|| control_rebo4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite300_tag.Component->Message(20, 0.0);
			control_lite322_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 24;
			MissionControl(66, nullptr);
		}
	}
}

void control::TimeWarpPartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite55_tag.Component->Message(7, -1.0);
			control_lite26_tag.Component->Message(7, -1.0);
			control_lite304_tag.Component->Message(7, 0.0);
			control_lite317_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(147, 0), -1.0);
		return;
	}
	if (control_kickout2_tag.Component == caller)
	{
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(47, 0), 4.0);
		if (control_middle_circle_tag.Component->Message(37, 0.0) > 1)
		{
			control_middle_circle_tag.Component->Message(33, 5.0);
			int rank = control_middle_circle_tag.Component->Message(37, 0.0);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(174, 0), pinball::get_rc_string(RankRcArray[rank - 1], 1));
			control_mission_text_box_tag.Component->Display(Buffer, 8.0);
		}
	}
	else
	{
		if (control_ramp_tag.Component != caller)
			return;
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(46, 0), 4.0);
		if (control_middle_circle_tag.Component->Message(37, 0.0) < 9)
		{
			int rank = control_middle_circle_tag.Component->Message(37, 0.0);
			control_middle_circle_tag.Component->Message(41, 5.0);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(173, 0), pinball::get_rc_string(RankRcArray[rank], 1));
		}
		if (!AddRankProgress(12))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0);
			control_soundwave10_tag.Component->Play();
		}
	}
	SpecialAddScore(2000000);
	control_lite55_tag.Component->Message(20, 0.0);
	control_lite26_tag.Component->Message(20, 0.0);
	control_lite304_tag.Component->Message(20, 0.0);
	control_lite317_tag.Component->Message(20, 0.0);
	control_lite198_tag.Component->MessageField = 1;
	MissionControl(66, nullptr);
	// SpecialAddScore sets the score dirty flag. So next tick it will be redrawn.
}

void control::UnselectMissionController(int code, TPinballComponent* caller)
{
	control_lite198_tag.Component->MessageField = 1;
	MissionControl(66, nullptr);
}

void control::WaitingDeploymentController(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 63:
		if (control_oneway4_tag.Component == caller || control_oneway10_tag.Component == caller)
		{
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
		}
		break;
	case 66:
		control_mission_text_box_tag.Component->Clear();
		waiting_deployment_flag = 0;
		break;
	case 67:
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(50, 0), -1.0);
		break;
	default:
		break;
	}
}
