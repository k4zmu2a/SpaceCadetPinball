#include "pch.h"
#include "pb.h"

#include "memory.h"
#include "pinball.h"
#include "proj.h"
#include "render.h"
#include "loader.h"
#include "options.h"
#include "timer.h"

TPinballTable* pb::MainTable = nullptr;
datFileStruct* pb::record_table = nullptr;
int pb::time_ticks = 0, pb::demo_mode = 0;

int pb::init()
{
	float projMat[12], zMin = 0, zScaler = 0;
	CHAR datFileName[300];
	CHAR dataFilePath[300];

	++memory::critical_allocation;
	lstrcpyA(datFileName, pinball::DatFileName);
	pinball::make_path_name(dataFilePath, datFileName, 300);
	record_table = partman::load_records(dataFilePath);

	auto useBmpFont = 0;
	pinball::get_rc_int(158, &useBmpFont);
	if (useBmpFont)
		score::load_msg_font("pbmsg_ft");

	if (!record_table)
		return (int)&record_table->NumberOfGroups + 1;

	auto plt = (PALETTEENTRY*)partman::field_labeled(record_table, "background", datFieldTypes::Palette);
	gdrv::display_palette(plt);

	auto tableSize = (__int16*)partman::field_labeled(record_table, "table_size", datFieldTypes::ShortArray);
	auto backgroundBmp = (gdrv_bitmap8*)partman::field_labeled(record_table, "background", datFieldTypes::Bitmap8bit);
	auto cameraInfo = (float*)partman::field_labeled(record_table, "camera_info", datFieldTypes::FloatArray);

	if (cameraInfo)
	{
		memcpy(&projMat, cameraInfo, sizeof(float) * 4 * 3);
		cameraInfo += 12;

		auto projCenterX = tableSize[0] * 0.5f;
		auto projCenterY = tableSize[1] * 0.5f;
		auto projD = cameraInfo[0];
		proj::init(projMat, projD, projCenterX, projCenterY);
		zMin = cameraInfo[1];
		zScaler = cameraInfo[2];
	}

	render::init(nullptr, zMin, zScaler, tableSize[0], tableSize[1]);
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

	//high_score_read(highscore_table, (int)&pb_state);
	//v11 = *(float*)((char*)MainTable->ListP2.ListPtr->Array[0] + 154);
	//ball_speed_limit = v11 * 200.0;

	--memory::critical_allocation;
	return 0;
}

void pb::reset_table()
{
	if (MainTable)
		MainTable->Message(1024, 0.0);
}


void pb::firsttime_setup()
{
	render::blit = 0;
	render::update();
	render::blit = 1;
}

void pb::paint()
{
	render::paint();
}

void pb::mode_change(int mode)
{
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
	//if (options::Options.Music)
	//midi_play_pb_theme(0);
	MainTable->Message(1014, static_cast<float>(options::Options.Players));
}

void pb::ballset(int x, int y)
{	
}

int pb::frame(int time)
{
	return 1;
}