#pragma once

class font_selection
{
public:
	static void show_dialog();
	static void RenderDialog();
private:
	static bool ShowDialog;
	static char DialogInputBuffer[512];
};
