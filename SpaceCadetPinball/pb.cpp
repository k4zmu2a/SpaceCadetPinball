#include "pch.h"
#include "pb.h"


#include "control.h"
#include "fullscrn.h"
#include "high_score.h"
#include "proj.h"
#include "render.h"
#include "loader.h"
#include "midi.h"
#include "nudge.h"
#include "options.h"
#include "timer.h"
#include "winmain.h"
#include "Sound.h"
#include "TBall.h"
#include "TDemo.h"
#include "TEdgeSegment.h"
#include "TLightGroup.h"
#include "TPlunger.h"
#include "TTableLayer.h"
#include "GroupData.h"
#include "partman.h"
#include "score.h"
#include "TFlipper.h"
#include "TPinballTable.h"
#include "TTextBox.h"
#include "translations.h"

TPinballTable* pb::MainTable = nullptr;
DatFile* pb::record_table = nullptr;
int pb::time_ticks = 0;
GameModes pb::game_mode = GameModes::GameOver;
float pb::time_now = 0, pb::time_next = 0, pb::time_ticks_remainder = 0;
float pb::BallMaxSpeed, pb::BallHalfRadius, pb::BallToBallCollisionDistance;
float pb::IdleTimerMs = 0;
bool pb::FullTiltMode = false, pb::FullTiltDemoMode = false, pb::cheat_mode = false, pb::demo_mode = false, pb::CreditsActive = false;
std::string pb::DatFileName, pb::BasePath;
ImU32 pb::TextBoxColor;
int pb::quickFlag = 0;
TTextBox *pb::InfoTextBox, *pb::MissTextBox;


int pb::init()
{
	float projMat[12];

	if (DatFileName.empty())
		return 1;
	auto dataFilePath = make_path_name(DatFileName);
	record_table = partman::load_records(dataFilePath.c_str(), FullTiltMode);

	auto useBmpFont = 0;
	get_rc_int(Msg::TextBoxUseBitmapFont, &useBmpFont);
	if (useBmpFont)
		score::load_msg_font("pbmsg_ft");

	if (!record_table)
		return 1;

	auto plt = (ColorRgba*)record_table->field_labeled("background", FieldTypes::Palette);
	gdrv::display_palette(plt);

	auto backgroundBmp = record_table->GetBitmap(record_table->record_labeled("background"));
	auto cameraInfoId = record_table->record_labeled("camera_info") + fullscrn::GetResolution();
	auto cameraInfo = (float*)record_table->field(cameraInfoId, FieldTypes::FloatArray);

	/*Full tilt: table size depends on resolution*/
	auto resInfo = &fullscrn::resolution_array[fullscrn::GetResolution()];

	if (cameraInfo)
	{
		memcpy(&projMat, cameraInfo, sizeof(float) * 4 * 3);
		cameraInfo += 12;

		auto projCenterX = resInfo->TableWidth * 0.5f;
		auto projCenterY = resInfo->TableHeight * 0.5f;
		auto projD = cameraInfo[0];
		auto zMin = cameraInfo[1];
		auto zScaler = cameraInfo[2];
		proj::init(projMat, projD, projCenterX, projCenterY, zMin, zScaler);
	}

	render::init(nullptr, resInfo->TableWidth, resInfo->TableHeight);
	gdrv::copy_bitmap(
		render::vscreen,
		backgroundBmp->Width,
		backgroundBmp->Height,
		backgroundBmp->XPosition,
		backgroundBmp->YPosition,
		backgroundBmp,
		0,
		0);

	loader::loadfrom(record_table);

	mode_change(GameModes::InGame);

	time_ticks = 0;
	timer::init(150);
	score::init();

	MainTable = new TPinballTable();

	high_score::read();
	auto ball = MainTable->BallList.at(0);
	BallMaxSpeed = ball->Radius * 200.0f;
	BallHalfRadius = ball->Radius * 0.5f;
	BallToBallCollisionDistance = (ball->Radius + BallHalfRadius) * 2.0f;

	int red = 255, green = 255, blue = 255;
	auto fontColor = get_rc_string(Msg::TextBoxColor);
	if (fontColor)
		sscanf(fontColor, "%d %d %d", &red, &green, &blue);
	TextBoxColor = IM_COL32(red, green, blue, 255);

	return 0;
}

int pb::uninit()
{
	score::unload_msg_font();
	loader::unload();
	delete record_table;
	high_score::write();
	delete MainTable;
	MainTable = nullptr;
	timer::uninit();
	render::uninit();
	return 0;
}

void pb::SelectDatFile(const std::vector<const char*>& dataSearchPaths)
{
	DatFileName.clear();
	FullTiltDemoMode = FullTiltMode = false;

	std::string datFileNames[3]
	{
		"CADET.DAT",
		"PINBALL.DAT",
		"DEMO.DAT",
	};

	// Default game data test order: CADET.DAT, PINBALL.DAT, DEMO.DAT
	if (options::Options.Prefer3DPBGameData)
	{
		std::swap(datFileNames[0], datFileNames[1]);
	}
	for (auto path : dataSearchPaths)
	{
		if (!path)
			continue;

		BasePath = path;
		for (const auto& datFileName : datFileNames)
		{
			auto fileName = datFileName;
			for (int i = 0; i < 2; i++)
			{
				if (i == 1)
					std::transform(fileName.begin(), fileName.end(), fileName.begin(),
					               [](unsigned char c) { return std::tolower(c); });

				auto datFilePath = make_path_name(fileName);
				auto datFile = fopenu(datFilePath.c_str(), "r");
				if (datFile)
				{
					fclose(datFile);
					DatFileName = fileName;
					if (datFileName == "CADET.DAT")
						FullTiltMode = true;
					if (datFileName == "DEMO.DAT")
						FullTiltDemoMode = FullTiltMode = true;
					printf("Loading game from: %s\n", datFilePath.c_str());
					return;
				}
			}
		}
	}
}

void pb::reset_table()
{
	if (MainTable)
		MainTable->Message(MessageCode::Reset, 0.0);
}


void pb::firsttime_setup()
{
	render::update();
}

void pb::mode_change(GameModes mode)
{
	if (CreditsActive && MissTextBox)
		MissTextBox->Clear(true);
	CreditsActive = false;
	IdleTimerMs = 0;

	switch (mode)
	{
	case GameModes::InGame:
		if (demo_mode)
		{
			winmain::LaunchBallEnabled = false;
			winmain::HighScoresEnabled = false;
			winmain::DemoActive = true;
			if (MainTable)
			{
				if (MainTable->Demo)
					MainTable->Demo->ActiveFlag = 1;
			}
		}
		else
		{
			winmain::LaunchBallEnabled = true;
			winmain::HighScoresEnabled = true;
			winmain::DemoActive = false;
			if (MainTable)
			{
				if (MainTable->Demo)
					MainTable->Demo->ActiveFlag = 0;
			}
		}
		break;
	case GameModes::GameOver:
		winmain::LaunchBallEnabled = false;
		if (!demo_mode)
		{
			winmain::HighScoresEnabled = true;
			winmain::DemoActive = false;
		}
		if (MainTable && MainTable->LightGroup)
			MainTable->LightGroup->Message(MessageCode::TLightGroupGameOverAnimation, 1.4f);
		break;
	}
	game_mode = mode;
}

void pb::toggle_demo()
{
	if (demo_mode)
	{
		demo_mode = false;
		MainTable->Message(MessageCode::Reset, 0.0);
		mode_change(GameModes::GameOver);
		MissTextBox->Clear();
		InfoTextBox->Display(get_rc_string(Msg::STRING125), -1.0);
	}
	else
	{
		replay_level(true);
	}
}

void pb::replay_level(bool demoMode)
{
	demo_mode = demoMode;
	mode_change(GameModes::InGame);
	if (options::Options.Music)
		midi::music_play();
	MainTable->Message(MessageCode::NewGame, static_cast<float>(options::Options.Players));
}

void pb::ballset(float dx, float dy)
{
	// dx and dy are normalized to window, ideally in [-1, 1]
	static constexpr float sensitivity = 7000;

	for (auto ball : MainTable->BallList)
	{
		if (ball->ActiveFlag)
		{
			ball->Direction.X = dx * sensitivity;
			ball->Direction.Y = dy * sensitivity;
			ball->Speed = maths::normalize_2d(ball->Direction);
			ball->LastActiveTime = time_ticks;
		}
	}
}

void pb::frame(float dtMilliSec)
{
	if (dtMilliSec > 100)
		dtMilliSec = 100;
	if (dtMilliSec <= 0)
		return;

	if (FullTiltMode && !demo_mode)
	{
		IdleTimerMs += dtMilliSec;
		if (IdleTimerMs >= 60000 && !CreditsActive)
		{
			PushCheat("credits");
		}
	}

	float dtSec = dtMilliSec * 0.001f;
	time_next = time_now + dtSec;
	timed_frame(dtSec);
	time_now = time_next;

	dtMilliSec += time_ticks_remainder;
	auto dtWhole = static_cast<int>(dtMilliSec);
	time_ticks_remainder = dtMilliSec - static_cast<float>(dtWhole);
	time_ticks += dtWhole;

	if (nudge::nudged_left || nudge::nudged_right || nudge::nudged_up)
	{
		nudge::nudge_count = dtSec * 4.0f + nudge::nudge_count;
	}
	else
	{
		auto nudgeDec = nudge::nudge_count - dtSec;
		if (nudgeDec <= 0.0f)
			nudgeDec = 0.0;
		nudge::nudge_count = nudgeDec;
	}
	timer::check();
	render::update();
	score::update(MainTable->CurScoreStruct);
	if (!MainTable->TiltLockFlag)
	{
		if (nudge::nudge_count > 0.5f)
		{
			InfoTextBox->Display(get_rc_string(Msg::STRING126), 2.0);
		}
		if (nudge::nudge_count > 1.0f)
			MainTable->tilt(time_now);
	}
}

void pb::timed_frame(float timeDelta)
{
	for (auto ball : MainTable->BallList)
	{
		if (!ball->ActiveFlag || ball->HasGroupFlag || ball->CollisionComp || ball->Speed >= 0.8f)
		{
			if (ball->StuckCounter > 0)
			{
				vector2 dist{ball->Position.X - ball->PrevPosition.X, ball->Position.Y - ball->PrevPosition.Y};
				auto radiusX2 = ball->Radius * 2.0f;
				if (radiusX2 * radiusX2 < maths::magnitudeSq(dist))
					ball->StuckCounter = 0;
			}
			ball->LastActiveTime = time_ticks;
		}
		else if (time_ticks - ball->LastActiveTime > 500)
		{
			vector2 dist{ball->Position.X - ball->PrevPosition.X, ball->Position.Y - ball->PrevPosition.Y};
			auto radiusD2 = ball->Radius / 2.0f;
			ball->PrevPosition = ball->Position;
			if (radiusD2 * radiusD2 < maths::magnitudeSq(dist))
				ball->StuckCounter = 0;
			else
				ball->StuckCounter++;
			control::UnstuckBall(*ball, time_ticks - ball->LastActiveTime);
		}
	}

	int ballSteps[20]{};
	float ballStepsDistance[20]{};
	int maxStep = -1;
	for (auto index = 0u; index < MainTable->BallList.size(); index++)
	{
		auto ball = MainTable->BallList[index];
		ballSteps[index] = -1;
		if (ball->ActiveFlag != 0)
		{
			vector2 vecDst{};
			ball->TimeDelta = timeDelta;
			if (ball->TimeDelta > 0.01f && ball->Speed < 0.8f)
				ball->TimeDelta = 0.01f;
			ball->CollisionDisabledFlag = false;
			if (ball->CollisionComp)
			{
				ball->CollisionComp->FieldEffect(ball, &vecDst);
			}
			else
			{
				TTableLayer::edge_manager->FieldEffects(ball, &vecDst);
				vecDst.X *= ball->TimeDelta;
				vecDst.Y *= ball->TimeDelta;
				ball->Direction.X *= ball->Speed;
				ball->Direction.Y *= ball->Speed;
				maths::vector_add(ball->Direction, vecDst);
				ball->Speed = maths::normalize_2d(ball->Direction);
				if (ball->Speed > BallMaxSpeed)
					ball->Speed = BallMaxSpeed;

				ballStepsDistance[index] = ball->Speed * ball->TimeDelta;
				auto ballStep = static_cast<int>(std::ceil(ballStepsDistance[index] / BallHalfRadius)) - 1;
				ballSteps[index] = ballStep;
				if (ballStep > maxStep)
					maxStep = ballStep;
			}
		}
	}

	float deltaAngle[4]{};
	int flipperSteps[4]{};
	for (auto index = 0u; index < MainTable->FlipperList.size(); index++)
	{
		auto flipStep = MainTable->FlipperList[index]->GetFlipperStepAngle(timeDelta, &deltaAngle[index]) - 1;
		flipperSteps[index] = flipStep;
		if (flipStep > maxStep)
			maxStep = flipStep;
	}

	ray_type ray{};
	ray.MinDistance = 0.002f;
	for (auto step = 0; step <= maxStep; step++)
	{
		for (auto ballIndex = 0u; ballIndex < MainTable->BallList.size(); ballIndex++)
		{
			auto ball = MainTable->BallList[ballIndex];
			if (!ball->CollisionDisabledFlag && ballSteps[ballIndex] >= step)
			{
				ray.CollisionMask = ball->CollisionMask;

				for (auto distanceSum = 0.0f; distanceSum < BallHalfRadius;)
				{
					ray.Origin = ball->Position;
					ray.Direction = ball->Direction;
					if (ballSteps[ballIndex] <= step)
					{
						ray.MaxDistance = ballStepsDistance[ballIndex] - ballSteps[ballIndex] * BallHalfRadius;
					}
					else
					{
						ray.MaxDistance = BallHalfRadius;
					}

					TEdgeSegment* edge = nullptr;
					auto distance = TTableLayer::edge_manager->FindCollisionDistance(&ray, ball, &edge);
					if (distance > 0.0f)
					{
						distance = BallToBallCollision(ray, *ball, &edge, distance);
					}
					if (ball->EdgeCollisionResetFlag)
					{
						ball->EdgeCollisionResetFlag = false;
					}
					else
					{
						ball->EdgeCollisionCount = 0;
						ball->EdgeCollisionResetFlag = true;
					}
					if (distance >= 1e9f)
					{
						ball->Position.X += ray.MaxDistance * ray.Direction.X;
						ball->Position.Y += ray.MaxDistance * ray.Direction.Y;
						break;
					}

					edge->EdgeCollision(ball, distance);
					if (distance <= 0.0f || ball->CollisionDisabledFlag)
						break;
					distanceSum += distance;
				}
			}
		}

		for (auto flipIndex = 0u; flipIndex < MainTable->FlipperList.size(); flipIndex++)
		{
			if (flipperSteps[flipIndex] >= step)
				MainTable->FlipperList[flipIndex]->FlipperCollision(deltaAngle[flipIndex]);
		}
	}

	for (const auto flipper : MainTable->FlipperList)
	{
		flipper->UpdateSprite();
	}

	for (auto ball : MainTable->BallList)
	{
		if (ball->ActiveFlag)
			ball->Repaint();
	}
}

void pb::pause_continue()
{
	winmain::single_step ^= true;
	InfoTextBox->Clear();
	MissTextBox->Clear();
	if (winmain::single_step)
	{
		if (MainTable)
			MainTable->Message(MessageCode::Pause, time_now);
		InfoTextBox->Display(get_rc_string(Msg::STRING123), -1.0);
		midi::music_stop();
		Sound::Deactivate();
	}
	else
	{
		if (MainTable)
			MainTable->Message(MessageCode::Resume, 0.0);
		if (!demo_mode)
		{
			const char* text;
			float textTime;
			if (game_mode == GameModes::GameOver)
			{
				textTime = -1.0;
				text = get_rc_string(Msg::STRING125);
			}
			else
			{
				textTime = 5.0;
				text = get_rc_string(Msg::STRING124);
			}
			InfoTextBox->Display(text, textTime);
		}
		if (options::Options.Music && !winmain::single_step)
			midi::music_play();
		Sound::Activate();
	}
}

void pb::loose_focus()
{
	if (MainTable)
		MainTable->Message(MessageCode::LooseFocus, time_now);
}

void pb::InputUp(GameInput input)
{
	if (game_mode != GameModes::InGame || winmain::single_step || demo_mode)
		return;

	const auto bindings = options::MapGameInput(input);
	for (const auto binding : bindings)
	{
		switch (binding)
		{
		case GameBindings::LeftFlipper:
			MainTable->Message(MessageCode::LeftFlipperInputReleased, time_now);
			break;
		case GameBindings::RightFlipper:
			MainTable->Message(MessageCode::RightFlipperInputReleased, time_now);
			break;
		case GameBindings::Plunger:
			MainTable->Message(MessageCode::PlungerInputReleased, time_now);
			break;
		case GameBindings::LeftTableBump:
			nudge::un_nudge_right(0, nullptr);
			break;
		case GameBindings::RightTableBump:
			nudge::un_nudge_left(0, nullptr);
			break;
		case GameBindings::BottomTableBump:
			nudge::un_nudge_up(0, nullptr);
			break;
		default: break;
		}
	}
}

void pb::InputDown(GameInput input)
{
	if (options::WaitingForInput())
	{
		options::InputDown(input);
		return;
	}

	const auto bindings = options::MapGameInput(input);
	for (const auto binding : bindings)
	{
		winmain::HandleGameBinding(binding, true);
	}

	if (game_mode != GameModes::InGame || winmain::single_step || demo_mode)
		return;

	if (CreditsActive)
		MissTextBox->Clear(true);
	CreditsActive = false;
	IdleTimerMs = 0;

	if (input.Type == InputTypes::Keyboard)
		control::pbctrl_bdoor_controller(static_cast<char>(input.Value));

	for (const auto binding : bindings)
	{
		switch (binding)
		{
		case GameBindings::LeftFlipper:
			MainTable->Message(MessageCode::LeftFlipperInputPressed, time_now);
			break;
		case GameBindings::RightFlipper:
			MainTable->Message(MessageCode::RightFlipperInputPressed, time_now);
			break;
		case GameBindings::Plunger:
			MainTable->Message(MessageCode::PlungerInputPressed, time_now);
			break;
		case GameBindings::LeftTableBump:
			if (!MainTable->TiltLockFlag)
				nudge::nudge_right();
			break;
		case GameBindings::RightTableBump:
			if (!MainTable->TiltLockFlag)
				nudge::nudge_left();
			break;
		case GameBindings::BottomTableBump:
			if (!MainTable->TiltLockFlag)
				nudge::nudge_up();
			break;
		default: break;
		}
	}

	if (cheat_mode && input.Type == InputTypes::Keyboard)
	{
		switch (input.Value)
		{
		case 'b':
			{
				vector2 pos{6.0f, 7.0f};
				if (!MainTable->BallCountInRect(pos, MainTable->CollisionCompOffset * 1.2f) && MainTable->AddBall(pos))
					MainTable->MultiballCount++;
				break;
			}
		case 'h':
			{
				high_score_struct entry{{0}, 1000000000};
				strncpy(entry.Name, get_rc_string(Msg::STRING127), sizeof entry.Name - 1);
				high_score::show_and_set_high_score_dialog({entry, 1});
				break;
			}
		case 'r':
			control::cheat_bump_rank();
			break;
		case 's':
			MainTable->AddScore(static_cast<int>(RandFloat() * 1000000.0f));
			break;
		case SDLK_F12:
			MainTable->port_draw();
			break;
		case 'i':
			MainTable->LightGroup->Message(MessageCode::TLightFtTmpOverrideOn, 1.0f);
			break;
		case 'j':
			MainTable->LightGroup->Message(MessageCode::TLightFtTmpOverrideOff, 1.0f);
			break;
		}
	}
}

void pb::launch_ball()
{
	MainTable->Plunger->Message(MessageCode::PlungerLaunchBall, 0.0f);
}

void pb::end_game()
{
	int scores[4]{};
	int scoreIndex[4]{};

	mode_change(GameModes::GameOver);
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
		for (auto j = i + 1; j < playerCount; ++j)
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
			int position = high_score::get_score_position(scores[i]);
			if (position >= 0)
			{
				high_score_struct entry{{0}, scores[i]};
				const char* playerName;

				switch (scoreIndex[i])
				{
				default:
				case 0: playerName = get_rc_string(Msg::STRING127);
					break;
				case 1: playerName = get_rc_string(Msg::STRING128);
					break;
				case 2: playerName = get_rc_string(Msg::STRING129);
					break;
				case 3: playerName = get_rc_string(Msg::STRING130);
					break;
				}

				strncpy(entry.Name, playerName, sizeof entry.Name - 1);
				high_score::show_and_set_high_score_dialog({entry, -1});
			}
		}
	}
}

void pb::high_scores()
{
	high_score::show_high_score_dialog();
}

void pb::tilt_no_more()
{
	if (MainTable->TiltLockFlag)
		InfoTextBox->Clear();
	MainTable->TiltLockFlag = 0;
	nudge::nudge_count = -2.0;
}

bool pb::chk_highscore()
{
	if (demo_mode)
		return false;
	for (auto i = 0; i < MainTable->PlayerCount; ++i)
	{
		if (high_score::get_score_position(MainTable->PlayerScores[i].ScoreStruct->Score) >= 0)
			return true;
	}
	return false;
}

void pb::PushCheat(const std::string& cheat)
{
	for (auto ch : cheat)
		control::pbctrl_bdoor_controller(ch);
}

LPCSTR pb::get_rc_string(Msg uID)
{
	return translations::GetTranslation(uID);
}

int pb::get_rc_int(Msg uID, int* dst)
{
	*dst = atoi(get_rc_string(uID));
	return 1;
}

std::string pb::make_path_name(const std::string& fileName)
{
	return BasePath + fileName;
}

void pb::ShowMessageBox(Uint32 flags, LPCSTR title, LPCSTR message)
{
	fprintf(flags == SDL_MESSAGEBOX_ERROR ? stderr : stdout, "BL error: %s\n%s\n", title, message);
	SDL_ShowSimpleMessageBox(flags, title, message, winmain::MainWindow);
}

float pb::BallToBallCollision(const ray_type& ray, const TBall& ball, TEdgeSegment** edge, float collisionDistance)
{
	for (const auto curBall : MainTable->BallList)
	{
		if (curBall->ActiveFlag && curBall != &ball && (curBall->CollisionMask & ball.CollisionMask) != 0 &&
			std::abs(curBall->Position.X - ball.Position.X) < BallToBallCollisionDistance &&
			std::abs(curBall->Position.Y - ball.Position.Y) < BallToBallCollisionDistance)
		{
			auto distance = curBall->FindCollisionDistance(ray);
			if (distance < 1e9f)
			{
				distance = std::max(0.0f, distance - 0.002f);
				if (distance < collisionDistance)
				{
					collisionDistance = distance;
					*edge = curBall;
				}
			}
		}
	}

	return collisionDistance;
}
