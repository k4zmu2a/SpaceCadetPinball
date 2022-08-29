#pragma once

class font_selection
{
public:
	static void ShowDialog();
	static void RenderDialog();
private:
	static bool ShowDialogFlag;
	static char DialogInputBuffer[512];
};
