#include "pch.h"
#include "high_score.h"

#include "memory.h"
#include "options.h"
#include "pinball.h"
#include "score.h"

int high_score::dlg_enter_name;
int high_score::dlg_score;
int high_score::dlg_position;
char high_score::default_name[32]{};
high_score_struct* high_score::dlg_hst;
bool high_score::ShowDialog = false;


int high_score::read(high_score_struct* table)
{
	char Buffer[20];

	int checkSum = 0;
	clear_table(table);
	for (auto position = 0; position < 5; ++position)
	{
		auto tablePtr = &table[position];

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Name");
		auto name = options::get_string(Buffer, "");
		strncpy(tablePtr->Name, name.c_str(), sizeof tablePtr->Name);

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Score");
		tablePtr->Score = options::get_int(Buffer, tablePtr->Score);

		for (int i = static_cast<int>(strlen(tablePtr->Name)); --i >= 0; checkSum += tablePtr->Name[i])
		{
		}
		checkSum += tablePtr->Score;
	}

	auto verification = options::get_int("Verification", 7);
	if (checkSum != verification)
		clear_table(table);
	return 0;
}

int high_score::write(high_score_struct* table)
{
	char Buffer[20];

	int checkSum = 0;
	for (auto position = 0; position < 5; ++position)
	{
		auto tablePtr = &table[position];

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Name");
		options::set_string(Buffer, tablePtr->Name);

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Score");
		options::set_int(Buffer, tablePtr->Score);

		for (int i = static_cast<int>(strlen(tablePtr->Name)); --i >= 0; checkSum += tablePtr->Name[i])
		{
		}
		checkSum += tablePtr->Score;
	}

	options::set_int("Verification", checkSum);
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
		if (strlen(scoreStr) >= 31)
			scoreStr[31] = 0;
		strncpy(posTable->Name, scoreStr, sizeof posTable->Name);
		posTable->Name[31] = 0;
	}
	return position;
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
	strncpy(default_name, defaultName, sizeof default_name - 1);
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
				snprintf(buf, sizeof buf, "%d", row);
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
