#include "pch.h"
#include "TTextBox.h"

#include "control.h"
#include "fullscrn.h"
#include "loader.h"
#include "pb.h"
#include "render.h"
#include "score.h"
#include "timer.h"


TTextBox::TTextBox(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
{
	OffsetX = 0;
	OffsetY = 0;
	Width = 0;
	Height = 0;
	BgBmp = render::background_bitmap;
	Font = score::msg_fontp;
	Message1 = nullptr;
	Message2 = nullptr;
	Timer = 0;

	if (groupIndex > 0)
	{
		/*Full tilt: text box dimensions index is offset by resolution*/
		int arrLength;
		auto dimensions = loader::query_iattribute(groupIndex + fullscrn::GetResolution(), 1500, &arrLength);
		OffsetX = dimensions[0];
		OffsetY = dimensions[1];
		Width = dimensions[2];
		Height = dimensions[3];
	}
}

TTextBox::~TTextBox()
{
	if (Timer)
	{
		if (Timer != -1)
			timer::kill(Timer);
		Timer = 0;
	}
	while (Message1)
	{
		TTextBoxMessage* message = Message1;
		TTextBoxMessage* nextMessage = message->NextMessage;
		delete message;
		Message1 = nextMessage;
	}
}

int TTextBox::Message(int code, float value)
{
	return 0;
}

void TTextBox::TimerExpired(int timerId, void* caller)
{
	auto tb = static_cast<TTextBox*>(caller);
	TTextBoxMessage* message = tb->Message1;
	tb->Timer = 0;
	if (message)
	{
		TTextBoxMessage* nextMessage = message->NextMessage;
		delete message;
		tb->Message1 = nextMessage;
		tb->Draw();
		control::handler(60, tb);
	}
}

void TTextBox::Clear()
{
	gdrv_bitmap8* bmp = BgBmp;
	if (bmp)
		gdrv::copy_bitmap(
			render::vscreen,
			Width,
			Height,
			OffsetX,
			OffsetY,
			bmp,
			OffsetX,
			OffsetY);
	else
		gdrv::fill_bitmap(render::vscreen, Width, Height, OffsetX, OffsetY, 0);
	if (Timer)
	{
		if (Timer != -1)
			timer::kill(Timer);
		Timer = 0;
	}
	while (Message1)
	{
		TTextBoxMessage* message = Message1;
		TTextBoxMessage* nextMessage = message->NextMessage;
		delete message;
		Message1 = nextMessage;
	}
}

void TTextBox::Display(const char* text, float time)
{
	if (!text)
		return;

	if (Message1 && !strcmp(text, Message2->Text))
	{
		Message2->Refresh(time);
		if (Message2 == Message1)
		{
			if (Timer && Timer != -1)
				timer::kill(Timer);
			if (time == -1.0f)
				Timer = -1;
			else
				Timer = timer::set(time, this, TimerExpired);
		}
	}
	else
	{
		if (Timer == -1)
			Clear();

		auto message = new TTextBoxMessage(text, time);
		if (message)
		{
			if (message->Text)
			{
				if (Message1)
					Message2->NextMessage = message;
				else
					Message1 = message;
				Message2 = message;
				if (Timer == 0)
					Draw();
			}
			else
			{
				delete message;
			}
		}
	}
}

void TTextBox::Draw()
{
	auto bmp = BgBmp;
	if (bmp)
		gdrv::copy_bitmap(
			render::vscreen,
			Width,
			Height,
			OffsetX,
			OffsetY,
			bmp,
			OffsetX,
			OffsetY);
	else
		gdrv::fill_bitmap(render::vscreen, Width, Height, OffsetX, OffsetY, 0);

	bool display = false;
	while (Message1)
	{
		if (Message1->Time == -1.0f)
		{
			if (!Message1->NextMessage)
			{
				Timer = -1;
				display = true;
				break;
			}
		}
		else if (Message1->TimeLeft() >= -2.0f)
		{
			Timer = timer::set(std::max(Message1->TimeLeft(), 0.25f), this, TimerExpired);
			display = true;
			break;
		}

		auto tmp = Message1;
		Message1 = Message1->NextMessage;
		delete tmp;
	}

	if (display)
	{
		if (!Font)
		{
			// Handled by gdrv::grtext_draw_ttext_in_box()
			return;
		}

		std::vector<LayoutResult> lines{};
		auto textHeight = 0;
		for (auto text = Message1->Text; ; textHeight += Font->Height)
		{
			if (!text[0] || textHeight + Font->Height > Height)
				break;

			auto line = LayoutTextLine(text);
			if (line.Start == line.End)
				break;
			lines.push_back(line);
			text = line.End;
		}

		// Textboxes in FT display texts centered
		auto offY = OffsetY;
		if (pb::FullTiltMode)
			offY += (Height - textHeight) / 2;
		for (auto line : lines)
		{
			auto offX = OffsetX;
			if (pb::FullTiltMode)
				offX += (Width - line.Width) / 2;
			for (auto text = line.Start; text < line.End; text++)
			{
				auto charBmp = Font->Chars[*text & 0x7F];
				if (charBmp)
				{
					auto height = charBmp->Height;
					auto width = charBmp->Width;
					if (render::background_bitmap)
						gdrv::copy_bitmap_w_transparency(render::vscreen, width, height, offX, offY, charBmp, 0,
						                                 0);
					else
						gdrv::copy_bitmap(render::vscreen, width, height, offX, offY, charBmp, 0, 0);
					offX += charBmp->Width + Font->GapWidth;
				}
			}
			offY += Font->Height;
		}
	}
}

TTextBox::LayoutResult TTextBox::LayoutTextLine(char* textStart) const
{
	auto lineWidth = 0, wordWidth = 0;
	char *wordBoundary = nullptr, *textEnd;
	for (textEnd = textStart; ; ++textEnd)
	{
		auto maskedChar = textEnd[0] & 0x7F;
		if (!maskedChar || maskedChar == '\n')
			break;

		auto charBmp = Font->Chars[maskedChar];
		if (!charBmp)
			continue;

		auto width = lineWidth + charBmp->Width + Font->GapWidth;
		if (width > Width)
		{
			if (wordBoundary)
			{
				textEnd = wordBoundary;
				lineWidth = wordWidth;
			}
			break;
		}
		if (maskedChar == ' ')
		{
			wordBoundary = textEnd;
			wordWidth = width;
		}
		lineWidth = width;
	}

	while ((*textEnd & 0x7F) == ' ')
		++textEnd;
	if ((*textEnd & 0x7F) == '\n')
		++textEnd;
	return LayoutResult{textStart, textEnd, lineWidth};
}
