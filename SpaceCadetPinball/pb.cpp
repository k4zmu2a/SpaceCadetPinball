#include "pch.h"
#include "pb.h"


#include "control.h"
#include "fullscrn.h"
#include "high_score.h"
#include "memory.h"
#include "pinball.h"
#include "proj.h"
#include "render.h"
#include "loader.h"
#include "midi.h"
#include "nudge.h"
#include "objlist_class.h"
#include "options.h"
#include "timer.h"
#include "winmain.h"
#include "resource.h"
#include "TBall.h"
#include "TDemo.h"
#include "TEdgeSegment.h"
#include "TLightGroup.h"
#include "TPlunger.h"
#include "TTableLayer.h"

TPinballTable* pb::MainTable = nullptr;
datFileStruct* pb::record_table = nullptr;
int pb::time_ticks = 0, pb::demo_mode = 0, pb::game_mode = 2, pb::mode_countdown_, pb::state, pb::frameCounter = 0;
float pb::time_now, pb::time_next, pb::ball_speed_limit;
high_score_struct pb::highscore_table[5];
bool pb::FullTiltMode = false, pb::cheat_mode = false;


int pb::init()
{
	float projMat[12], zMin = 0, zScaler = 0;
	CHAR datFileName[300];
	CHAR dataFilePath[300];

	++memory::critical_allocation;
	lstrcpyA(datFileName, winmain::DatFileName);
	pinball::make_path_name(dataFilePath, datFileName, 300);
	record_table = partman::load_records(dataFilePath, fullscrn::GetResolution(), FullTiltMode);

	auto useBmpFont = 0;
	pinball::get_rc_int(158, &useBmpFont);
	if (useBmpFont)
		score::load_msg_font("pbmsg_ft");

	if (!record_table)
		return 1;

	auto plt = (PALETTEENTRY*)partman::field_labeled(record_table, "background", datFieldTypes::Palette);
	gdrv::display_palette(plt);

	auto backgroundBmp = (gdrv_bitmap8*)partman::field_labeled(record_table, "background", datFieldTypes::Bitmap8bit);
	auto cameraInfoId = partman::record_labeled(record_table, "camera_info") + fullscrn::GetResolution();
	auto cameraInfo = (float*)partman::field(record_table, cameraInfoId, datFieldTypes::FloatArray);

	/*Full tilt: table size depends on resolution*/
	auto resInfo = &fullscrn::resolution_array[fullscrn::GetResolution()];

	if (cameraInfo)
	{
		memcpy(&projMat, cameraInfo, sizeof(float) * 4 * 3);
		cameraInfo += 12;

		auto projCenterX = resInfo->TableWidth * 0.5f;
		auto projCenterY = resInfo->TableHeight * 0.5f;
		auto projD = cameraInfo[0];
		proj::init(projMat, projD, projCenterX, projCenterY);
		zMin = cameraInfo[1];
		zScaler = cameraInfo[2];
	}

	render::init(nullptr, zMin, zScaler, resInfo->TableWidth, resInfo->TableHeight);
	gdrv::copy_bitmap(
		&render::vscreen,
		backgroundBmp->Width,
		backgroundBmp->Height,
		backgroundBmp->XPosition,
		backgroundBmp->YPosition,
		backgroundBmp,
		0,
		0);

	gdrv::destroy_bitmap(backgroundBmp);
	loader::loadfrom(record_table);

	if (pinball::quickFlag)
		mode_change(1);
	else
		mode_change(3);

	time_ticks = 0;
	timer::init(150);
	score::init();

	MainTable = new TPinballTable();

	high_score::read(highscore_table, &state);
	ball_speed_limit = MainTable->BallList->Get(0)->Offset * 200.0f;
	--memory::critical_allocation;
	return 0;
}

int pb::uninit()
{
	score::unload_msg_font();
	loader::unload();
	partman::unload_records(record_table);
	high_score::write(highscore_table, &state);
	if (MainTable)
		delete MainTable;
	MainTable = nullptr;
	gdrv::get_focus();
	timer::uninit();
	render::uninit();
	return 0;
}

void pb::reset_table()
{
	if (MainTable)
		MainTable->Message(1024, 0.0);
}


void pb::firsttime_setup()
{
	render::update(false);
}

void pb::paint()
{
	render::paint();
}

void pb::mode_change(int mode)
{
	switch (mode)
	{
	case 1:
		if (demo_mode)
		{
			options::menu_set(Menu1_Launch_Ball, 0);
			options::menu_set(Menu1_High_Scores, 0);
			options::menu_check(Menu1_Demo, 1);
			if (MainTable)
			{
				if (MainTable->Demo)
					MainTable->Demo->ActiveFlag = 1;
			}
		}
		else
		{
			options::menu_set(Menu1_High_Scores, 1);
			options::menu_set(Menu1_Launch_Ball, 1);
			options::menu_check(Menu1_Demo, 0);
			if (MainTable)
			{
				if (MainTable->Demo)
					MainTable->Demo->ActiveFlag = 0;
			}
		}
		break;
	case 2:
		options::menu_set(Menu1_Launch_Ball, 0);
		if (!demo_mode)
		{
			options::menu_set(Menu1_High_Scores, 1);
			options::menu_check(Menu1_Demo, 0);
		}
		if (MainTable && MainTable->LightGroup)
			MainTable->LightGroup->Message(29, 1.4f);
		break;
	case 3:
	case 4:
		options::menu_set(Menu1_Launch_Ball, 0);
		options::menu_set(Menu1_High_Scores, 0);
		mode_countdown_ = 5000;
		break;
	}
	game_mode = mode;
}

void pb::toggle_demo()
{
	if (demo_mode)
	{
		demo_mode = 0;
		MainTable->Message(1024, 0.0);
		mode_change(2);
		pinball::MissTextBox->Clear();
		auto text = pinball::get_rc_string(24, 0);
		pinball::InfoTextBox->Display(text, -1.0);
	}
	else
	{
		replay_level(1);
	}
}

void pb::replay_level(int demoMode)
{
	demo_mode = demoMode;
	mode_change(1);
	if (options::Options.Music)
		midi::play_pb_theme(0);
	MainTable->Message(1014, static_cast<float>(options::Options.Players));
}

void pb::ballset(int x, int y)
{
	TBall* ball = MainTable->BallList->Get(0);
	ball->Acceleration.X = x * 30.0f;
	ball->Acceleration.Y = y * 30.0f;
	ball->Speed = maths::normalize_2d(&ball->Acceleration);
}

int pb::frame(int time)
{
	static int frameTime = 0;

	if (time > 100)
		time = 100;
	float timeMul = time * 0.001f;
	if (!mode_countdown(time))
	{
		time_next = time_now + timeMul;
		timed_frame(time_now, timeMul, true);
		time_now = time_next;
		time_ticks += time;
		if (nudge::nudged_left || nudge::nudged_right || nudge::nudged_up)
		{
			nudge::nudge_count = timeMul * 4.0f + nudge::nudge_count;
		}
		else
		{
			auto nudgeDec = nudge::nudge_count - timeMul;
			if (nudgeDec <= 0.0f)
				nudgeDec = 0.0;
			nudge::nudge_count = nudgeDec;
		}
		timer::check();

		if (!options::Options.AlternativeRender)
		{
			render::update(true);
		}
		else
		{
			// Screen update at UPS > screen refresh rate cause tearing.
			// Especially noticeable on fast moving ball in scaled up window.
			// Retained render prevents frame skip. The next best thing - complete refresh at fixed rate. 
			render::update(false);

			// Frame time at 60 FPS = 16.(6) ms = (16 + 17 + 17) / 3
			auto targetTime = frameCounter % 3 == 0 ? 16 : 17;
			frameTime += time;
			if (frameTime >= targetTime)
			{
				frameTime = min(frameTime - targetTime, 100);
				render::shift(0, 0, 0, 0, MainTable->Width, MainTable->Height);
				frameCounter++;
			}
		}

		score::update(MainTable->CurScoreStruct);
		if (!MainTable->TiltLockFlag)
		{
			if (nudge::nudge_count > 0.5f)
			{
				pinball::InfoTextBox->Display(pinball::get_rc_string(25, 0), 2.0);
			}
			if (nudge::nudge_count > 1.0f)
				MainTable->tilt(time_now);
		}
	}
	return 1;
}

void pb::timed_frame(float timeNow, float timeDelta, bool drawBalls)
{
	vector_type vec1{}, vec2{};

	for (int i = 0; i < MainTable->BallList->GetCount(); i++)
	{
		auto ball = MainTable->BallList->Get(i);
		if (ball->ActiveFlag != 0)
		{
			auto collComp = ball->CollisionComp;
			if (collComp)
			{
				ball->TimeDelta = timeDelta;
				collComp->FieldEffect(ball, &vec1);
			}
			else
			{
				if (MainTable->ActiveFlag)
				{
					vec2.X = 0.0;
					vec2.Y = 0.0;
					vec2.Z = 0.0;
					TTableLayer::edge_manager->FieldEffects(ball, &vec2);
					vec2.X = vec2.X * timeDelta;
					vec2.Y = vec2.Y * timeDelta;
					ball->Acceleration.X = ball->Speed * ball->Acceleration.X;
					ball->Acceleration.Y = ball->Speed * ball->Acceleration.Y;
					maths::vector_add(&ball->Acceleration, &vec2);
					ball->Speed = maths::normalize_2d(&ball->Acceleration);
					ball->InvAcceleration.X = ball->Acceleration.X == 0.0f ? 1.0e9f : 1.0f / ball->Acceleration.X;
					ball->InvAcceleration.Y = ball->Acceleration.Y == 0.0f ? 1.0e9f : 1.0f / ball->Acceleration.Y;
				}

				auto timeDelta2 = timeDelta;
				auto timeNow2 = timeNow;
				for (auto index = 10; timeDelta2 > 0.000001f && index; --index)
				{
					auto time = collide(timeNow2, timeDelta2, ball);
					timeDelta2 -= time;
					timeNow2 += time;
				}
			}
		}
	}

	if (drawBalls)
	{
		for (int i = 0; i < MainTable->BallList->GetCount(); i++)
		{
			auto ball = MainTable->BallList->Get(i);
			if (ball->ActiveFlag)
				ball->Repaint();
		}
	}
}

void pb::window_size(int* width, int* height)
{
	*width = fullscrn::resolution_array[fullscrn::GetResolution()].TableWidth;
	*height = fullscrn::resolution_array[fullscrn::GetResolution()].TableHeight;
}

void pb::pause_continue()
{
	winmain::single_step = winmain::single_step == 0;
	pinball::InfoTextBox->Clear();
	pinball::MissTextBox->Clear();
	if (winmain::single_step)
	{
		if (MainTable)
			MainTable->Message(1008, time_now);
		pinball::InfoTextBox->Display(pinball::get_rc_string(22, 0), -1.0);
		midi::music_stop();
	}
	else
	{
		if (MainTable)
			MainTable->Message(1009, 0.0);
		if (!demo_mode)
		{
			char* text;
			float textTime;
			if (game_mode == 2)
			{
				textTime = -1.0;
				text = pinball::get_rc_string(24, 0);
			}
			else
			{
				textTime = 5.0;
				text = pinball::get_rc_string(23, 0);
			}
			pinball::InfoTextBox->Display(text, textTime);
		}
		if (options::Options.Music && !winmain::single_step)
			midi::play_pb_theme(0);
	}
}

void pb::loose_focus()
{
	if (MainTable)
		MainTable->Message(1010, time_now);
}

void pb::keyup(int key)
{
	if (game_mode == 1 && !winmain::single_step && !demo_mode)
	{
		if (key == options::Options.LeftFlipperKey)
		{
			MainTable->Message(1001, time_now);
		}
		else if (key == options::Options.RightFlipperKey)
		{
			MainTable->Message(1003, time_now);
		}
		else if (key == options::Options.PlungerKey)
		{
			MainTable->Message(1005, time_now);
		}
		else if (key == options::Options.LeftTableBumpKey)
		{
			nudge::un_nudge_right(0, nullptr);
		}
		else if (key == options::Options.RightTableBumpKey)
		{
			nudge::un_nudge_left(0, nullptr);
		}
		else if (key == options::Options.BottomTableBumpKey)
		{
			nudge::un_nudge_up(0, nullptr);
		}
	}
}

void pb::keydown(int key)
{
	if (winmain::single_step || demo_mode)
		return;
	if (game_mode != 1)
	{
		mode_countdown(-1);
		return;
	}
	control::pbctrl_bdoor_controller(static_cast<char>(key));
	if (key == options::Options.LeftFlipperKey)
	{
		MainTable->Message(1000, time_now);
		return;
	}
	if (key == options::Options.RightFlipperKey)
	{
		MainTable->Message(1002, time_now);
	}
	else
	{
		if (key == options::Options.PlungerKey)
		{
			MainTable->Message(1004, time_now);
			return;
		}
		if (key == options::Options.LeftTableBumpKey)
		{
			if (!MainTable->TiltLockFlag)
				nudge::nudge_right();
			return;
		}
		if (key == options::Options.RightTableBumpKey)
		{
			if (!MainTable->TiltLockFlag)
				nudge::nudge_left();
			return;
		}
		if (key == options::Options.BottomTableBumpKey)
		{
			if (!MainTable->TiltLockFlag)
				nudge::nudge_up();
			return;
		}
	}
	if (cheat_mode)
	{
		switch (key)
		{
		case 'B':
			TBall* ball;
			if (MainTable->BallList->GetCount() <= 0)
			{
				ball = new TBall(MainTable);
			}
			else
			{
				for (auto index = 0; ;)
				{
					ball = MainTable->BallList->Get(index);
					if (!ball->ActiveFlag)
						break;
					++index;
					if (index >= MainTable->BallList->GetCount())
					{
						ball = new TBall(MainTable);
						break;
					}
				}
			}
			ball->Position.X = 1.0;
			ball->ActiveFlag = 1;
			ball->Position.Z = ball->Offset;
			ball->Position.Y = 1.0;
			ball->Acceleration.Z = 0.0;
			ball->Acceleration.Y = 0.0;
			ball->Acceleration.X = 0.0;
			break;
		case 'H':
			char String1[200];
			lstrcpyA(String1, pinball::get_rc_string(26, 0));
			high_score::show_and_set_high_score_dialog(highscore_table, 1000000000, 1, String1);
			break;
		case 'M':
			char buffer[20];
			sprintf_s(buffer, "%zu", memory::use_total);
			MessageBoxA(winmain::hwnd_frame, buffer, "Mem:", 0x2000u);
			break;
		case 'R':
			control::cheat_bump_rank();
			break;
		case VK_F11:
			gdrv::get_focus();
			break;
		case VK_F12:
			MainTable->port_draw();
			break;
		}
	}
}

int pb::mode_countdown(int time)
{
	if (!game_mode || game_mode <= 0)
		return 1;
	if (game_mode > 2)
	{
		if (game_mode == 3)
		{
			mode_countdown_ -= time;
			if (mode_countdown_ < 0 || time < 0)
				mode_change(4);
		}
		else if (game_mode == 4)
		{
			mode_countdown_ -= time;
			if (mode_countdown_ < 0 || time < 0)
				mode_change(1);
		}
		return 1;
	}
	return 0;
}

void pb::launch_ball()
{
	MainTable->Plunger->Message(1017, 0.0f);
}

void pb::end_game()
{
	int scores[4]{};
	int scoreIndex[4]{};
	char String1[200];

	mode_change(2);
	int playerCount = MainTable->PlayerCount;

	score_struct_super* scorePtr = MainTable->PlayerScores;
	for (auto index = 0; index < playerCount; ++index)
	{
		scores[index] = scorePtr->ScoreStruct->Score;
		scoreIndex[index] = index;
		++scorePtr;
	}

	for (auto i = 0; i < playerCount; ++i)
	{
		for (auto j = i; j < playerCount; ++j)
		{
			if (scores[j] > scores[i])
			{
				int score = scores[j];
				scores[j] = scores[i];
				scores[i] = score;

				int index = scoreIndex[j];
				scoreIndex[j] = scoreIndex[i];
				scoreIndex[i] = index;
			}
		}
	}

	if (!demo_mode && !MainTable->CheatsUsed)
	{
		for (auto i = 0; i < playerCount; ++i)
		{
			int position = high_score::get_score_position(highscore_table, scores[i]);
			if (position >= 0)
			{
				lstrcpyA(String1, pinball::get_rc_string(scoreIndex[i] + 26, 0));
				high_score::show_and_set_high_score_dialog(highscore_table, scores[i], position, String1);
			}
		}
	}
}

void pb::high_scores()
{
	high_score::show_high_score_dialog(highscore_table);
}

void pb::tilt_no_more()
{
	if (MainTable->TiltLockFlag)
		pinball::InfoTextBox->Clear();
	MainTable->TiltLockFlag = 0;
	nudge::nudge_count = -2.0;
}

bool pb::chk_highscore()
{
	if (demo_mode)
		return false;
	int playerIndex = MainTable->PlayerCount - 1;
	if (playerIndex < 0)
		return false;
	for (int i = playerIndex;
	     high_score::get_score_position(highscore_table, MainTable->PlayerScores[i].ScoreStruct->Score) < 0;
	     --i)
	{
		if (--playerIndex < 0)
			return false;
	}
	return true;
}

float pb::collide(float timeNow, float timeDelta, TBall* ball)
{
	ray_type ray{};
	vector_type positionMod{};

	if (ball->ActiveFlag && !ball->CollisionComp)
	{
		if (ball_speed_limit < ball->Speed)
			ball->Speed = ball_speed_limit;

		auto maxDistance = timeDelta * ball->Speed;
		ball->TimeDelta = timeDelta;
		ball->RayMaxDistance = maxDistance;
		ball->TimeNow = timeNow;

		ray.Origin.X = ball->Position.X;
		ray.Origin.Y = ball->Position.Y;
		ray.Origin.Z = ball->Position.Z;
		ray.Direction.X = ball->Acceleration.X;
		ray.Direction.Y = ball->Acceleration.Y;
		ray.Direction.Z = ball->Acceleration.Z;
		ray.MaxDistance = maxDistance;
		ray.FieldFlag = ball->FieldFlag;
		ray.TimeNow = timeNow;
		ray.TimeDelta = timeDelta;
		ray.MinDistance = 0.0020000001f;

		TEdgeSegment* edge = nullptr;
		auto distance = TTableLayer::edge_manager->FindCollisionDistance(&ray, ball, &edge);
		ball->EdgeCollisionCount = 0;
		if (distance >= 1000000000.0f)
		{
			maxDistance = timeDelta * ball->Speed;
			ball->RayMaxDistance = maxDistance;
			positionMod.X = maxDistance * ball->Acceleration.X;
			positionMod.Y = maxDistance * ball->Acceleration.Y;
			positionMod.Z = 0.0;
			maths::vector_add(&ball->Position, &positionMod);
		}
		else
		{
			edge->EdgeCollision(ball, distance);
			if (ball->Speed > 0.000000001f)
				return fabs(distance / ball->Speed);
		}
	}
	return timeDelta;
}
