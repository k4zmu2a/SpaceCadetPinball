#pragma once
#include "score.h"
#include "TPinballComponent.h"
#include "TTextBoxMessage.h"

class TTextBox :
	public TPinballComponent
{
public:
	int OffsetX;
	int OffsetY;
	int Width;
	int Height;
	int Timer;
	gdrv_bitmap8* BgBmp;
	score_msg_font_type* Font;
	TTextBoxMessage* Message1;
	TTextBoxMessage* Message2;

	TTextBox(TPinballTable* table, int groupIndex);
	~TTextBox() override;
	int Message(int code, float value) override;
	void Clear();
	void Display(const wchar_t* text, float time);
	void Draw(bool redraw = false);

private:
	struct LayoutResult
	{
		wchar_t *Start, *End;
		int Width;
	};

	static void TimerExpired(int timerId, void* caller);

	
	LayoutResult LayoutTextLine(wchar_t* textStart) const;
};
