#pragma once
class TTextBoxMessage
{
public:
	TTextBoxMessage* NextMessage;
	wchar_t* Text;
	float Time;
	int EndTicks;

	TTextBoxMessage(const wchar_t* text, float time);
	~TTextBoxMessage();
	float TimeLeft() const;
	void Refresh(float time);
};
