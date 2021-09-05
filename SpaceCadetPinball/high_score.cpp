#include "pch.h"
#include "high_score.h"

#include "fullscrn.h"
#include "memory.h"
#include "options.h"
#include "resource.h"
#include "winmain.h"

int high_score::dlg_enter_name;
int high_score::dlg_score;
int high_score::dlg_position;
char high_score::default_name[32]{};
high_score_struct* high_score::dlg_hst;
bool high_score::ShowDialog = false;

winhelp_entry high_score::help[21]
{
	winhelp_entry{0x70, 0x3E9},
	winhelp_entry{0x191, 0x3EB},
	winhelp_entry{0x1F5, 0x3EB},
	winhelp_entry{0x259, 0x3EB},
	winhelp_entry{0x192, 0x3EB},
	winhelp_entry{0x193, 0x3EB},
	winhelp_entry{0x194, 0x3EB},
	winhelp_entry{0x195, 0x3EB},
	winhelp_entry{0x1F6, 0x3EB},
	winhelp_entry{0x1F7, 0x3EB},
	winhelp_entry{0x1F8, 0x3EB},
	winhelp_entry{0x1F9, 0x3EB},
	winhelp_entry{0x2BD, 0x3EB},
	winhelp_entry{0x2BE, 0x3EB},
	winhelp_entry{0x2BF, 0x3EB},
	winhelp_entry{0x2C0, 0x3EB},
	winhelp_entry{0x2C1, 0x3EB},
	winhelp_entry{0x2C2, 0x3EB},
	winhelp_entry{0x2C3, 0x3EB},
	winhelp_entry{0x2C4, 0x3EB},
	winhelp_entry{0, 0},
};

int high_score::read(high_score_struct* table, int* ptrToSmth)
{
	char Buffer[20];

	int scoreSum = 0;
	clear_table(table);
	char* buf1 = memory::allocate(300u);
	if (!buf1)
		return 1;
	char* buf2 = memory::allocate(300u);
	auto optPath = pinball::get_rc_string(166, 0);
	for (auto position = 0; position < 5; ++position)
	{
		auto tablePtr = &table[position];
		_itoa_s(position, Buffer, 10);
		lstrcatA(Buffer, ".Name");
		options::get_string(optPath, Buffer, buf1, "", 32);
		buf1[32] = 0;
		lstrcpyA(tablePtr->Name, buf1);
		_itoa_s(position, Buffer, 10);
		lstrcatA(Buffer, ".Score");
		options::get_string(optPath, Buffer, buf1, "", 300);
		tablePtr->Score = atol(buf1);
		for (int i = lstrlenA(tablePtr->Name); --i >= 0; scoreSum += tablePtr->Name[i])
		{
		}
		scoreSum += tablePtr->Score;
	}

	scramble_number_string(scoreSum, buf1);
	options::get_string(optPath, "Verification", buf2, "", 300);
	if (lstrcmpA(buf1, buf2))
		clear_table(table);
	memory::free(buf1);
	memory::free(buf2);
	return 0;
}

int high_score::write(high_score_struct* table, int* ptrToSmth)
{
	char Buffer[20];

	high_score_struct* tablePtr = table;
	int scoreSum = 0;
	CHAR* buf = memory::allocate(300u);
	if (!buf)
		return 1;
	const CHAR* optPath = pinball::get_rc_string(166, 0);
	for (auto position = 0; position < 5; ++position)
	{
		_itoa_s(position, Buffer, 10);
		lstrcatA(Buffer, ".Name");
		options::set_string(optPath, Buffer, tablePtr->Name);
		_itoa_s(position, Buffer, 10);
		lstrcatA(Buffer, ".Score");
		_ltoa_s(tablePtr->Score, buf, 300, 10);
		options::set_string(optPath, Buffer, buf);
		for (int i = lstrlenA(tablePtr->Name); --i >= 0; scoreSum += tablePtr->Name[i])
		{
		}
		scoreSum += tablePtr->Score;
		++position;
		++tablePtr;
	}
	scramble_number_string(scoreSum, buf);
	options::set_string(optPath, "Verification", buf);
	memory::free(buf);
	return 0;
}

void high_score::clear_table(high_score_struct* table)
{
	for (int index = 5; index; --index)
	{
		table->Score = -999;
		table->Name[0] = 0;
		++table;
	}
}

int high_score::get_score_position(high_score_struct* table, int score)
{
	if (score <= 0)
		return -1;

	for (int position = 0; position < 5; position++)
	{
		if (table[position].Score < score)
			return position;
	}
	return -1;
}

int high_score::place_new_score_into(high_score_struct* table, int score, LPSTR scoreStr, int position)
{
	if (position >= 0)
	{
		if (position <= 4)
		{
			high_score_struct* tablePtr = table + 4;
			int index = 5 - position;
			do
			{
				--index;
				memcpy(tablePtr, &tablePtr[-1], sizeof(high_score_struct));
				--tablePtr;
			}
			while (index);
		}
		high_score_struct* posTable = &table[position];
		posTable->Score = score;
		if (lstrlenA(scoreStr) >= 31)
			scoreStr[31] = 0;
		lstrcpyA(posTable->Name, scoreStr);
		posTable->Name[31] = 0;
	}
	return position;
}

void high_score::scramble_number_string(int Value, char* Buffer)
{
	_ltoa_s(Value, Buffer, 300, 10);
}

void high_score::show_high_score_dialog(high_score_struct* table)
{
	dlg_enter_name = 0;
	dlg_score = 0;
	dlg_hst = table;
	ShowDialog = true;
}

void high_score::show_and_set_high_score_dialog(high_score_struct* table, int score, int pos, LPCSTR defaultName)
{
	dlg_position = pos;
	dlg_score = score;
	dlg_hst = table;
	dlg_enter_name = 1;
	strncpy_s(default_name, defaultName, 32);
	ShowDialog = true;
}

void high_score::RenderHighScoreDialog()
{
	if (ShowDialog == true)
	{
		ShowDialog = false;
		if (dlg_position == -1) 
		{
			dlg_enter_name = 0;
			return;
		}
		ImGui::OpenPopup("High Scores");
	}

	bool unused_open = true;
	if (ImGui::BeginPopupModal("High Scores", &unused_open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::BeginTable("table1", 3, 0))
		{
			char buf[36];
			ImGui::TableSetupColumn("Rank");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Score");
			ImGui::TableHeadersRow();

			high_score_struct* tablePtr = dlg_hst;
			for (int row = 0; row < 5; row++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				_itoa_s(row, buf, 10);
				ImGui::TextUnformatted(buf);

				auto score = tablePtr->Score;
				ImGui::TableNextColumn();				
				if (dlg_enter_name == 1 && dlg_position == row)
				{
					score = dlg_score;
					ImGui::PushItemWidth(200);
					ImGui::InputText("", default_name, IM_ARRAYSIZE(default_name));
				}
				else
				{
					ImGui::TextUnformatted(tablePtr->Name);
				}

				ImGui::TableNextColumn();
				score::string_format(score, buf);
				ImGui::TextUnformatted(buf);

				tablePtr++;
			}
			ImGui::EndTable();
		}
		ImGui::Separator();

		if (ImGui::Button("Ok"))
		{
			if (dlg_enter_name)
			{				
				default_name[31] = 0;
				place_new_score_into(dlg_hst, dlg_score, default_name, dlg_position);				
			}
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			ImGui::OpenPopup("Confirm");
		if (ImGui::BeginPopupModal("Confirm", nullptr, ImGuiWindowFlags_MenuBar))
		{
			ImGui::TextUnformatted(pinball::get_rc_string(40, 0));
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				clear_table(dlg_hst);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::EndPopup();
	}
}
