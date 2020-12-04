#include "pch.h"
#include "TTextBox.h"
#include "loader.h"
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
		int arrLength;
		auto dimensions = loader::query_iattribute(groupIndex, 1500, &arrLength);
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
		control_handler(60, tb);
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
	gdrv::blit(
		&render::vscreen,
		OffsetX,
		OffsetY,
		OffsetX + render::vscreen.XPosition,
		OffsetY + render::vscreen.YPosition,
		Width,
		Height);
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
			if (time == -1.0)
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
	TTextBoxMessage* nextMessage = nullptr;
	auto this2 = this;

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

	while (this2->Message1)
	{
		auto message = this2->Message1;
		if (message->Time == -1.0)
		{
			nextMessage = message->NextMessage;
			if (!message->NextMessage)
			{
				this2->Timer = -1;
			LABEL_18:
				auto font = this2->Font;
				if (!font)
				{
					gdrv::blit(
						&render::vscreen,
						this2->OffsetX,
						this2->OffsetY,
						this2->OffsetX + render::vscreen.XPosition,
						this2->OffsetY + render::vscreen.YPosition,
						this2->Width,
						this2->Height);
					gdrv::grtext_draw_ttext_in_box(
						this2->Message1->Text,
						render::vscreen.XPosition + this2->OffsetX,
						render::vscreen.YPosition + this2->OffsetY,
						this2->Width,
						this2->Height,
						255);
					return;
				}
			}
		}
		else
		{
			auto timeLeft = (this2->Message1->TimeLeft());
			if (timeLeft >= -2.0f)
			{
				int timer;
				if (timeLeft >= 0.25f)
				{
					timer = timer::set(timeLeft, this2, TimerExpired);
				}
				else
				{
					timer = timer::set(0.25, this2, TimerExpired);
				}
				this2->Timer = timer;
				goto LABEL_18;
			}
			nextMessage = message->NextMessage;
		}
		delete message;
		this2->Message1 = nextMessage;
	}

	gdrv::blit(
		&render::vscreen,
		this2->OffsetX,
		this2->OffsetY,
		this2->OffsetX + render::vscreen.XPosition,
		this2->OffsetY + render::vscreen.YPosition,
		this2->Width,
		this2->Height);
}
