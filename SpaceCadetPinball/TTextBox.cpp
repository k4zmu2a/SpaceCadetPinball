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
			&render::vscreen,
			Width,
			Height,
			OffsetX,
			OffsetY,
			bmp,
			OffsetX,
			OffsetY);
	else
		gdrv::fill_bitmap(&render::vscreen, Width, Height, OffsetX, OffsetY, 0);	
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

void TTextBox::Display(char* text, float time)
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
			&render::vscreen,
			Width,
			Height,
			OffsetX,
			OffsetY,
			bmp,
			OffsetX,
			OffsetY);
	else
		gdrv::fill_bitmap(&render::vscreen, Width, Height, OffsetX, OffsetY, 0);

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
		auto font = Font;
		if (!font)
		{
			gdrv::grtext_draw_ttext_in_box(
				Message1->Text,
				render::vscreen.XPosition + OffsetX,
				render::vscreen.YPosition + OffsetY,
				Width,
				Height,
				255);
			return;
		}

		auto text = Message1->Text;
		for (auto y = OffsetY; ; y += font->Height)
		{
			auto curChar = *text;
			if (!curChar || y + font->Height > OffsetY + Height)
				break;

			auto totalWidth = 0;
			char* textEndSpace = nullptr;
			auto textEnd = text;
			while (true)
			{
				auto maskedChar = curChar & 0x7F;
				if (!maskedChar || maskedChar == '\n')
					break;
				auto charBmp = font->Chars[maskedChar];
				if (charBmp)
				{
					auto width = charBmp->Width + font->GapWidth + totalWidth;
					if (width > Width)
					{
						if (textEndSpace)
							textEnd = textEndSpace;
						break;
					}
					if (*textEnd == ' ')
						textEndSpace = textEnd;
					curChar = *(textEnd + 1);
					totalWidth = width;
					++textEnd;
				}
				else
				{
					curChar = *textEnd;
				}
			}

			auto offX = OffsetX;
			while (text < textEnd)
			{
				auto charBmp = font->Chars[*text++ & 0x7F];
				if (charBmp)
				{
					auto height = charBmp->Height;
					auto width = charBmp->Width;
					if (render::background_bitmap)
						gdrv::copy_bitmap_w_transparency(&render::vscreen, width, height, offX, y, charBmp, 0,
						                                 0);
					else
						gdrv::copy_bitmap(&render::vscreen, width, height, offX, y, charBmp, 0, 0);
					font = Font;
					offX += charBmp->Width + font->GapWidth;
				}
			}
			while ((*text & 0x7F) == ' ')
				++text;
			if ((*text & 0x7F) == '\n')
				++text;
		}
	}	
}
