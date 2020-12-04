#include "pch.h"
#include "pb.h"

#include "memory.h"
#include "pinball.h"
#include "proj.h"
#include "render.h"
#include "loader.h"
#include "midi.h"
#include "nudge.h"
#include "options.h"
#include "timer.h"
#include "winmain.h"

TPinballTable* pb::MainTable = nullptr;
datFileStruct* pb::record_table = nullptr;
int pb::time_ticks = 0, pb::demo_mode = 0, pb::cheat_mode = 0, pb::game_mode = 2, pb::mode_countdown_, pb::
    ball_speed_limit;
float pb::time_now, pb::time_next;

int pb::init()
{
	float projMat[12], zMin = 0, zScaler = 0;
	CHAR datFileName[300];
	CHAR dataFilePath[300];

	++memory::critical_allocation;
	lstrcpyA(datFileName, winmain::DatFileName);
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
	gdrv::fill_bitmap(&render::vscreen, render::vscreen.Width, render::vscreen.Height, 0, 0,
	                  static_cast<char>(0xff)); // temp
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

int pb::uninit()
{
	score::unload_msg_font();
	loader::unload();
	partman::unload_records(record_table);
	//high_score_write(highscore_table, (int)&pb_state);
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
	midi::play_pb_theme(0);
	MainTable->Message(1014, static_cast<float>(options::Options.Players));
}

void pb::ballset(int x, int y)
{
}

int pb::frame(int time)
{
	if (time > 100)
		time = 100;
	float timeMul = time * 0.001f;
	if (!mode_countdown(time))
	{
		time_next = time_now + timeMul;
		//pb::timed_frame(time_now, timeMul, 1);
		time_now = time_next;
		time_ticks += time;
		/*if (nudged_left || nudged_right || nudged_up)
		{
			nudge_count = timeMul * 4.0 + nudge_count;
		}
		else
		{
			v2 = nudge_count - timeMul;
			if (v2 <= 0.0)
				v2 = 0.0;
			nudge_count = v2;
		}*/
		timer::check();
		render::update();
		//score::update(MainTable->Score1);
		/*if (!MainTable->UnknownP83)
		{
			if (nudge_count > 0.5)
			{
				v3 = pinball:: get_rc_string(25, 0);
				pinball::InfoTextBox->Display( v3, 2.0);
			}
			if (nudge_count > 1.0)
				TPinballTable::tilt(MainTable, v1, time_now);
		}*/
	}
	return 1;
}

void pb::window_size(int* width, int* height)
{
	*width = 600;
	*height = 416;
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
			nudge::un_nudge_right(0, 0);
		}
		else if (key == options::Options.RightTableBumpKey)
		{
			nudge::un_nudge_left(0, 0);
		}
		else if (key == options::Options.BottomTableBumpKey)
		{
			nudge::un_nudge_up(0, 0);
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
	ctrl_bdoor_controller(key);
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
			if (!MainTable->UnknownP83)
				nudge::nudge_right();
			return;
		}
		if (key == options::Options.RightTableBumpKey)
		{
			if (!MainTable->UnknownP83)
				nudge::nudge_left();
			return;
		}
		if (key == options::Options.BottomTableBumpKey)
		{
			if (!MainTable->UnknownP83)
				nudge::nudge_up();
			return;
		}
	}
	if (cheat_mode)
	{
		switch (key)
		{
		case 'B':
			/**/
			break;
		case 'H':
			/*auto v1 = get_rc_string(26, 0);
			lstrcpyA(&String1, v1);
			show_and_set_high_score_dialog(highscore_table, 1000000000, 1, &String1);*/
			break;
		case 'M':
			char buffer[20];
			sprintf_s(buffer, "%ld", memory::use_total);
			MessageBoxA(winmain::hwnd_frame, buffer, "Mem:", 0x2000u);
			break;
		case 'R':
			cheat_bump_rank();
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

void pb::ctrl_bdoor_controller(int key)
{
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

int pb::cheat_bump_rank()
{
	return 0;
}

void pb::launch_ball()
{
}

int pb::end_game()
{
	return 0;
}

void pb::high_scores()
{
}
