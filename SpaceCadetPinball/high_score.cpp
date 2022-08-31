#include "pch.h"
#include "high_score.h"

#include "options.h"
#include "pinball.h"
#include "score.h"
#include "translations.h"

bool high_score::dlg_enter_name;
bool high_score::ShowDialog = false;
high_score_entry high_score::DlgData;
std::vector<high_score_entry> high_score::ScoreQueue;
high_score_struct high_score::highscore_table[5];

int high_score::read()
{
	char Buffer[20];

	int checkSum = 0;
	clear_table();
	for (auto position = 0; position < 5; ++position)
	{
		auto& tablePtr = highscore_table[position];

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Name");
		auto name = options::get_string(Buffer, "");
		strncpy(tablePtr.Name, name.c_str(), sizeof tablePtr.Name);

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Score");
		tablePtr.Score = options::get_int(Buffer, tablePtr.Score);

		for (int i = static_cast<int>(strlen(tablePtr.Name)); --i >= 0; checkSum += tablePtr.Name[i])
		{
		}
		checkSum += tablePtr.Score;
	}

	auto verification = options::get_int("Verification", 7);
	if (checkSum != verification)
		clear_table();
	return 0;
}

int high_score::write()
{
	char Buffer[20];

	int checkSum = 0;
	for (auto position = 0; position < 5; ++position)
	{
		auto& tablePtr = highscore_table[position];

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Name");
		options::set_string(Buffer, tablePtr.Name);

		snprintf(Buffer, sizeof Buffer, "%d", position);
		strcat(Buffer, ".Score");
		options::set_int(Buffer, tablePtr.Score);

		for (int i = static_cast<int>(strlen(tablePtr.Name)); --i >= 0; checkSum += tablePtr.Name[i])
		{
		}
		checkSum += tablePtr.Score;
	}

	options::set_int("Verification", checkSum);
	return 0;
}

void high_score::clear_table()
{
	for (auto& table : highscore_table)
	{
		table.Score = -999;
		table.Name[0] = 0;
	}
}

int high_score::get_score_position(int score)
{
	if (score <= 0)
		return -1;

	for (int position = 0; position < 5; position++)
	{
		if (highscore_table[position].Score < score)
			return position;
	}
	return -1;
}

void high_score::place_new_score_into(high_score_entry data)
{
	if (data.Position >= 0 && data.Position < 5)
	{
		for (int i = 4; i > data.Position; i--)
		{
			highscore_table[i] = highscore_table[i - 1];
		}

		data.Entry.Name[31] = 0;
		highscore_table[data.Position] = data.Entry;
	}
}

void high_score::show_high_score_dialog()
{
	ShowDialog = true;
}

void high_score::show_and_set_high_score_dialog(high_score_entry score)
{
	ScoreQueue.insert(ScoreQueue.begin(), score);
	ShowDialog = true;
}

void high_score::RenderHighScoreDialog()
{
	if (ShowDialog == true)
	{
		ShowDialog = false;
		if (!ImGui::IsPopupOpen(pinball::get_rc_string(Msg::HIGHSCORES_Caption)))
		{
			dlg_enter_name = false;
			while (!ScoreQueue.empty())
			{
				DlgData = ScoreQueue.back();
				ScoreQueue.pop_back();
				if (DlgData.Position < 0 || DlgData.Position > 4)
				{
					DlgData.Position = get_score_position(DlgData.Entry.Score);
				}

				if (DlgData.Position != -1)
				{
					dlg_enter_name = true;
					break;
				}
			}

			ImGui::OpenPopup(pinball::get_rc_string(Msg::HIGHSCORES_Caption));
		}
	}

	bool unused_open = true;
	if (ImGui::BeginPopupModal(pinball::get_rc_string(Msg::HIGHSCORES_Caption), &unused_open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::BeginTable("table1", 3, ImGuiTableFlags_Borders))
		{
			char buf[36];
			ImGui::TableSetupColumn(pinball::get_rc_string(Msg::HIGHSCORES_Rank));
			ImGui::TableSetupColumn(pinball::get_rc_string(Msg::HIGHSCORES_Name));
			ImGui::TableSetupColumn(pinball::get_rc_string(Msg::HIGHSCORES_Score));
			ImGui::TableHeadersRow();

			for (int offset = 0, row = 0; row < 5; row++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				snprintf(buf, sizeof buf, "%d", row + 1);
				ImGui::TextUnformatted(buf);

				auto currentRow = &highscore_table[row + offset];
				auto score = currentRow->Score;
				ImGui::TableNextColumn();
				if (dlg_enter_name && DlgData.Position == row)
				{
					offset = -1;
					score = DlgData.Entry.Score;
					ImGui::PushItemWidth(200);
					ImGui::InputText("", DlgData.Entry.Name, IM_ARRAYSIZE(DlgData.Entry.Name));
				}
				else
				{
					ImGui::TextUnformatted(currentRow->Name);
				}

				ImGui::TableNextColumn();
				score::string_format(score, buf);
				ImGui::TextUnformatted(buf);
			}
			ImGui::EndTable();
		}

		if (ImGui::Button(pinball::get_rc_string(Msg::HIGHSCORES_Ok)))
		{
			if (dlg_enter_name)
			{
				place_new_score_into(DlgData);
			}
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button(pinball::get_rc_string(Msg::HIGHSCORES_Cancel)))
			ImGui::CloseCurrentPopup();

		ImGui::SameLine();
		if (ImGui::Button(pinball::get_rc_string(Msg::HIGHSCORES_Clear)))
			ImGui::OpenPopup("Confirm");
		if (ImGui::BeginPopupModal("Confirm", nullptr, ImGuiWindowFlags_MenuBar))
		{
			ImGui::TextUnformatted(pinball::get_rc_string(Msg::STRING141));
			if (ImGui::Button(pinball::get_rc_string(Msg::HIGHSCORES_Ok), ImVec2(120, 0)))
			{
				clear_table();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button(pinball::get_rc_string(Msg::HIGHSCORES_Cancel), ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::EndPopup();

		// Reenter dialog for the next score in the queue
		if (!ImGui::IsPopupOpen(pinball::get_rc_string(Msg::HIGHSCORES_Caption)) && !ScoreQueue.empty())
		{
			ShowDialog = true;
		}
	}
}
