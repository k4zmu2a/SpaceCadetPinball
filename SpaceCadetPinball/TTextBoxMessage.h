#pragma once
class TTextBoxMessage
{
public:
	TTextBoxMessage* NextMessage;
	char* Text;
	float Time;
	int EndTicks;
	bool LowPriority;

	TTextBoxMessage(const char* text, float time, bool lowPriority);
	~TTextBoxMessage();
	float TimeLeft() const;
	void Refresh(float time);
};
