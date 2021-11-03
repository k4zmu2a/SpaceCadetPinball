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
LPCWSTR high_score::default_name;
high_score_struct* high_score::dlg_hst;

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
	char scoreBuffer[20];
	wchar_t nameBuffer[20];

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

		wsprintfW(nameBuffer, L"%d.Name", position);
		options::get_string(optPath, nameBuffer, tablePtr->Name, L"", 32);
		tablePtr->Name[31] = 0;

		sprintf_s(scoreBuffer, "%d.Score", position);
		options::get_string(optPath, scoreBuffer, buf1, "", 300);
		tablePtr->Score = atol(buf1);

		for (auto i = lstrlenW(tablePtr->Name) - 1; i >= 0; i--)
			scoreSum += tablePtr->Name[i];
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
	char scoreBuffer[20];
	wchar_t nameBuffer[20];

	int scoreSum = 0;
	CHAR* buf = memory::allocate(300u);
	if (!buf)
		return 1;
	auto optPath = pinball::get_rc_string(166, 0);
	for (auto position = 0; position < 5; ++position)
	{
		auto tablePtr = &table[position];

		wsprintfW(nameBuffer, L"%d.Name", position);
		options::set_string(optPath, nameBuffer, tablePtr->Name);

		sprintf_s(scoreBuffer, "%d.Score", position);
		_ltoa_s(tablePtr->Score, buf, 300, 10);
		options::set_string(optPath, scoreBuffer, buf);

		for (auto i = lstrlenW(tablePtr->Name) - 1; i >= 0; i--)
			scoreSum += tablePtr->Name[i];
		scoreSum += tablePtr->Score;
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

int high_score::place_new_score_into(high_score_struct* table, int score, LPWSTR name, int position)
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
		if (lstrlenW(name) >= 31)
			name[31] = 0;
		lstrcpyW(posTable->Name, name);
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
	DialogBoxParamW(winmain::hinst, L"dlg_highscores", winmain::hwnd_frame, HighScore, 0);
}

void high_score::show_and_set_high_score_dialog(high_score_struct* table, int score, int pos, LPCWSTR defaultName)
{
	dlg_position = pos;
	dlg_score = score;
	dlg_hst = table;
	dlg_enter_name = 1;
	default_name = defaultName;
	while (DialogBoxParamW(winmain::hinst, L"dlg_highscores", winmain::hwnd_frame, HighScore, 0))
	{
	}
}

INT_PTR high_score::HighScore(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND parent;
	int nIDDlgItem;
	wchar_t name[32];

	switch (msg)
	{
	case WM_CLOSE:
		SendMessageA(hWnd, WM_COMMAND, WM_DESTROY, 0);
		break;
	case WM_HELP:
		WinHelpA(static_cast<HWND>(reinterpret_cast<HELPINFO*>(lParam)->hItemHandle), "pinball.hlp", HELP_WM_HELP,
		         (ULONG_PTR)help);
		break;
	case WM_CONTEXTMENU:
		WinHelpA((HWND)wParam, "pinball.hlp", HELP_CONTEXTMENU, (ULONG_PTR)help);
		break;
	case WM_INITDIALOG:
		show_high_scores(hWnd, dlg_hst);
		for (nIDDlgItem = DLG_HIGHSCORES_EditName1; nIDDlgItem < 611; ++nIDDlgItem)
		{
			ShowWindow(GetDlgItem(hWnd, nIDDlgItem), SW_HIDE);
		}
		if (dlg_enter_name == 1)
		{
			if (dlg_position == -1)
			{
				dlg_enter_name = 0;
				return 1;
			}
			HWND nameTextBox = GetDlgItem(hWnd, dlg_position + DLG_HIGHSCORES_EditName1);
			ShowWindow(nameTextBox, SW_SHOW);
			EnableWindow(nameTextBox, 1);
			SetFocus(nameTextBox);
			if (default_name)
			{
				SetWindowTextW(nameTextBox, default_name);
				SendMessageA(nameTextBox, EM_SETSEL, 0, -1);
			}
			SendMessageA(nameTextBox, EM_SETLIMITTEXT, 31u, 0);
		}
		else
		{
			SetFocus(hWnd);
		}
		parent = GetParent(hWnd);
		if (parent)
			fullscrn::center_in(parent, hWnd);
		return 0;
	case WM_COMMAND:
		switch (wParam)
		{
		case DLG_HIGHSCORES_Ok:
			if (dlg_enter_name != 1)
			{
				break;
			}
			GetDlgItemTextW(hWnd, dlg_position + DLG_HIGHSCORES_EditName1, name, 32);
			name[31] = 0;
			place_new_score_into(dlg_hst, dlg_score, name, dlg_position);
			break;
		case DLG_HIGHSCORES_Cancel:
			break;
		case DLG_HIGHSCORES_Clear:
			if (MessageBoxW(hWnd, pinball::get_rc_Wstring(40, 0),
			                pinball::get_rc_Wstring(41, 0), MB_DEFBUTTON2 | MB_OKCANCEL) == 1)
			{
				clear_table(dlg_hst);
				if (dlg_enter_name)
					EndDialog(hWnd, 1);
				else
					EndDialog(hWnd, 0);
			}
			return 0;
		default:
			return 0;
		}

		dlg_enter_name = 0;
		EndDialog(hWnd, 0);
		return 1;
	}
	return 0;
}


void high_score::show_high_scores(HWND hDlg, high_score_struct* table)
{
	high_score_struct* tablePtr = table;
	int nextPosition = 0;
	for (int i = 0; i < 5; ++i)
	{
		if (dlg_enter_name == 1 && dlg_position == i)
		{
			hsdlg_show_score(hDlg, L" ", dlg_score, i);
			nextPosition = 1;
		}
		hsdlg_show_score(hDlg, tablePtr->Name, tablePtr->Score, i + nextPosition);
		++tablePtr;
	}
}

void high_score::hsdlg_show_score(HWND hDlg, LPCWSTR name, int score, int position)
{
	CHAR scoreStr[36];
	if (position < 5)
	{
		score::string_format(score, scoreStr);
		if (scoreStr[0])
		{
			SetWindowTextW(GetDlgItem(hDlg, position + DLG_HIGHSCORES_StaticName1), name);
			SetWindowTextA(GetDlgItem(hDlg, position + DLG_HIGHSCORES_Score1), scoreStr);
		}
	}
}
