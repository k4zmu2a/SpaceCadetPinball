#include "pch.h"
#include "TTextBoxMessage.h"
#include "memory.h"
#include "pb.h"

TTextBoxMessage::TTextBoxMessage(const char* text, float time)
{
	NextMessage = nullptr;
	Time = time;
	EndTicks = pb::time_ticks + static_cast<int>(time * 1000.0f);
	if (text)
	{
		const auto textLen = strlen(text) + 1;
		Text = memory::allocate(textLen);
		if (Text)
			strcpy_s(Text, textLen, text);
	}
	else
		Text = nullptr;
}

TTextBoxMessage::~TTextBoxMessage()
{
	if (Text)
		memory::free(Text);
}

float TTextBoxMessage::TimeLeft() const
{
	return static_cast<float>(EndTicks - pb::time_ticks) * 0.001f;
}

void TTextBoxMessage::Refresh(float time)
{
	this->Time = time;
	this->EndTicks = pb::time_ticks + static_cast<int>(time * 1000.0f);
}
