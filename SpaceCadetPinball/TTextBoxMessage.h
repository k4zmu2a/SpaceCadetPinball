#pragma once
class TTextBoxMessage
{
public:
	TTextBoxMessage* NextMessage;
	char* Text;
	float Time;
	int EndTicks;

	TTextBoxMessage(char* text, float time);
	~TTextBoxMessage();
	float TimeLeft() const;
	void Refresh(float time);
};
