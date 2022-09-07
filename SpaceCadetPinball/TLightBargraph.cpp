#include "pch.h"
#include "TLightBargraph.h"


#include "control.h"
#include "loader.h"
#include "timer.h"
#include "TPinballTable.h"

TLightBargraph::TLightBargraph(TPinballTable* table, int groupIndex) : TLightGroup(table, groupIndex)
{
	TimerTimeArray = nullptr;
	TLightBargraph::Reset();
	if (groupIndex > 0)
	{
		float* floatArr = loader::query_float_attribute(groupIndex, 0, 904);
		if (floatArr)
		{
			auto count = 2 * List.size();
			TimerTimeArray = new float[count];
			if (TimerTimeArray)
			{
				for (auto i = 0u; i < count; ++floatArr)
					TimerTimeArray[i++] = *floatArr;
			}
		}
	}
}

TLightBargraph::~TLightBargraph()
{
	delete[] TimerTimeArray;
}

int TLightBargraph::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TLightGroupGetOnCount:
		return TimeIndex;
	case MessageCode::TLightGroupToggleSplitIndex:
		{
			if (TimerBargraph)
			{
				timer::kill(TimerBargraph);
				TimerBargraph = 0;
			}
			auto timeIndex = static_cast<int>(floor(value));
			auto maxCount = static_cast<int>(List.size()) * 2;
			if (timeIndex >= maxCount)
				timeIndex = maxCount - 1;
			if (timeIndex >= 0)
			{
				TLightGroup::Message(MessageCode::TLightGroupToggleSplitIndex, static_cast<float>(timeIndex / 2));
				if (!(timeIndex & 1))
					TLightGroup::Message(MessageCode::TLightGroupStartFlasher, 0.0);
				if (TimerTimeArray)
					TimerBargraph = timer::set(TimerTimeArray[timeIndex], this, BargraphTimerExpired);
				TimeIndex = timeIndex;
			}
			else
			{
				TLightGroup::Message(MessageCode::TLightResetAndTurnOff, 0.0);
				TimeIndex = 0;
			}
			break;
		}
	case MessageCode::SetTiltLock:
		Reset();
		break;
	case MessageCode::PlayerChanged:
		if (TimerBargraph)
		{
			timer::kill(TimerBargraph);
			TimerBargraph = 0;
		}
		PlayerTimerIndexBackup[PinballTable->CurrentPlayer] = TimeIndex;
		Reset();
		TimeIndex = PlayerTimerIndexBackup[static_cast<int>(floor(value))];
		if (TimeIndex)
		{
			TLightBargraph::Message(MessageCode::TLightGroupToggleSplitIndex, static_cast<float>(TimeIndex));
		}
		break;
	case MessageCode::Reset:
		{
			Reset();
			int* playerPtr = PlayerTimerIndexBackup;
			for (auto index = 0; index < PinballTable->PlayerCount; ++index)
			{
				*playerPtr = TimeIndex;

				++playerPtr;
			}
			TLightGroup::Message(MessageCode::Reset, value);
			break;
		}
	default:
		TLightGroup::Message(code, value);
		break;
	}
	return 0;
}

void TLightBargraph::Reset()
{
	if (TimerBargraph)
	{
		timer::kill(TimerBargraph);
		TimerBargraph = 0;
	}
	TimeIndex = 0;
	TLightGroup::Reset();
}

void TLightBargraph::BargraphTimerExpired(int timerId, void* caller)
{
	auto bar = static_cast<TLightBargraph*>(caller);
	bar->TimerBargraph = 0;
	if (bar->TimeIndex)
	{
		bar->Message(MessageCode::TLightGroupToggleSplitIndex, static_cast<float>(bar->TimeIndex - 1));
		control::handler(MessageCode::ControlTimerExpired, bar);
	}
	else
	{
		bar->Message(MessageCode::TLightResetAndTurnOff, 0.0);
		control::handler(MessageCode::TLightGroupCountdownEnded, bar);
	}
}
