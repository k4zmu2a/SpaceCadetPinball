#include "pch.h"
#include "TLightGroup.h"


#include "control.h"
#include "loader.h"
#include "timer.h"
#include "TLight.h"
#include "TPinballTable.h"

TLightGroup::TLightGroup(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
{
	Timer = 0;
	NotifyTimer = 0;
	TLightGroup::Reset();
	if (groupIndex > 0)
	{
		int count;
		Timer1TimeDefault = *loader::query_float_attribute(groupIndex, 0, 903);
		int16_t* groupIndArr = loader::query_iattribute(groupIndex, 1027, &count);
		for (int index = 0; index < count; ++groupIndArr)
		{
			auto comp = dynamic_cast<TLight*>(table->find_component(*groupIndArr));
			if (comp)
				List.push_back(comp);
			++index;
		}
	}
}

int TLightGroup::Message(int code, float value)
{
	auto const count = static_cast<int>(List.size());
	switch (code)
	{
	case 1011:
	case 1022:
		break;
	case 1020:
		{
			auto playerPtr = &PlayerData[PinballTable->CurrentPlayer];
			playerPtr->MessageField = MessageField;
			playerPtr->MessageField2 = MessageField2;
			playerPtr->Timer1Time = Timer1Time;

			Reset();

			playerPtr = &PlayerData[static_cast<int>(floor(value))];
			MessageField = playerPtr->MessageField;
			MessageField2 = playerPtr->MessageField2;
			Timer1Time = playerPtr->Timer1Time;
			if (!(MessageField == 0))
				TimerExpired(0, this);
			break;
		}
	case 1024:
		Reset();
		for (auto index = 0; index < PinballTable->PlayerCount; index++)
		{
			auto playerPtr = &PlayerData[index];
			playerPtr->MessageField = MessageField;
			playerPtr->MessageField2 = MessageField2;
			playerPtr->Timer1Time = Timer1Time;
		}
		break;
	case 24:
		{
			auto lastLight = List.at(count - 1);
			if (lastLight->FlasherActive || lastLight->FlasherFlag2 || lastLight->FlasherFlag1)
				break;
			if (MessageField2)
			{
				TLightGroup::Message(34, 0.0);
			}
			AnimationFlag = 1;
			MessageField2 = code;
			auto lightMessageField = lastLight->MessageField;
			auto bmpIndex1 = lastLight->BmpIndex1;
			for (auto index = count - 1; index > 0; --index)
			{
				auto lightCur = List.at(index);
				auto lightPrev = List.at(index - 1);
				lightCur->Message(lightPrev->BmpIndex1 != 0, 0.0);
				lightCur->MessageField = lightPrev->MessageField;
			}
			auto firstLight = List.at(0);
			firstLight->Message(bmpIndex1 != 0, 0.0);
			firstLight->MessageField = lightMessageField;
			reschedule_animation(value);
			break;
		}
	case 25:
		{
			auto lastLight = List.at(count - 1);
			if (lastLight->FlasherActive || lastLight->FlasherFlag2 || lastLight->FlasherFlag1)
				break;
			if (MessageField2)
			{
				TLightGroup::Message(34, 0.0);
			}
			auto firstLight = List.at(0);
			AnimationFlag = 1;
			MessageField2 = code;
			auto lightMessageField = firstLight->MessageField;
			auto bmpIndex1 = firstLight->BmpIndex1;
			for (auto index = 0; index < count - 1; index++)
			{
				auto lightCur = List.at(index);
				auto lightNext = List.at(index + 1);
				lightCur->Message(lightNext->BmpIndex1 != 0, 0.0);
				lightCur->MessageField = lightNext->MessageField;
			}
			lastLight->Message(bmpIndex1 != 0, 0.0);
			lastLight->MessageField = lightMessageField;
			reschedule_animation(value);
			break;
		}
	case 26:
		{
			if (AnimationFlag || !MessageField2)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			auto lastLight = List.at(count - 1);
			auto flasherFlag2 = lastLight->FlasherFlag2;
			for (auto i = count - 1; i > 0; --i)
			{
				auto lightCur = List.at(i);
				auto lightPrev = List.at(i - 1);
				lightCur->Message((lightPrev->FlasherFlag2 != 0) + 8, 0.0);
			}
			auto firstLight = List.at(0);
			firstLight->Message((flasherFlag2 != 0) + 8, 0);
			reschedule_animation(value);
			break;
		}
	case 27:
		{
			if (AnimationFlag || !MessageField2)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			auto firstLight = List.at(0);
			auto flasherFlag2 = firstLight->FlasherFlag2;
			for (auto i = 0; i < count - 1; i++)
			{
				auto lightCur = List.at(i);
				auto lightNext = List.at(i + 1);
				lightCur->Message((lightNext->FlasherFlag2 != 0) + 8, 0.0);
			}
			auto lastLight = List.at(count - 1);
			lastLight->Message((flasherFlag2 != 0) + 8, 0);
			reschedule_animation(value);
			break;
		}
	case 28:
		{
			if (AnimationFlag || !MessageField2)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			for (auto light : List)
			{
				if (rand() % 100 > 70)
				{
					auto randVal = RandFloat() * value * 3.0f + 0.1f;
					light->Message(9, randVal);
				}
			}
			reschedule_animation(value);
			break;
		}
	case 29:
		{
			if (AnimationFlag || !MessageField2)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			for (auto light : List)
			{
				auto randVal = static_cast<float>(rand() % 100 > 70);
				light->Message(18, randVal);
			}
			reschedule_animation(value);
			break;
		}
	case 30:
		{
			auto noBmpInd1Count = 0;
			for (auto light : List)
			{
				if (!light->BmpIndex1)
					++noBmpInd1Count;
			}
			if (!noBmpInd1Count)
				break;

			auto randModCount = rand() % noBmpInd1Count;
			for (auto it = List.rbegin(); it != List.rend(); ++it)
			{
				auto light = *it;
				if (!light->BmpIndex1 && randModCount-- == 0)
				{
					light->Message(1, 0.0);
					break;
				}
			}

			if (MessageField2)
				start_animation();
			break;
		}
	case 31:
		{
			auto bmpInd1Count = 0;
			for (auto light : List)
			{
				if (light->BmpIndex1)
					++bmpInd1Count;
			}
			if (!bmpInd1Count)
				break;

			auto randModCount = rand() % bmpInd1Count;
			for (auto it = List.rbegin(); it != List.rend(); ++it)
			{
				auto light = *it;
				if (light->BmpIndex1 && randModCount-- == 0)
				{
					light->Message(0, 0.0);
					break;
				}
			}

			if (MessageField2)
				start_animation();
			break;
		}
	case 32:
		{
			auto index = next_light_up();
			if (index < 0)
				break;
			List.at(index)->Message(1, 0.0);
			if (MessageField2)
				start_animation();
			return 1;
		}
	case 33:
		{
			auto index = next_light_down();
			if (index < 0)
				break;
			List.at(index)->Message(0, 0.0);
			if (MessageField2)
				start_animation();
			return 1;
		}
	case 34:
		{
			if (Timer)
				timer::kill(Timer);
			Timer = 0;
			if (MessageField2 == 26 || MessageField2 == 27 || MessageField2 == 28)
				TLightGroup::Message(14, 0.0);
			MessageField2 = 0;
			AnimationFlag = 0;
			break;
		}
	case 35:
		{
			auto index = static_cast<int>(floor(value));
			if (index >= count || index < 0)
				break;

			auto light = List.at(index);
			light->Message(1, 0.0);
			if (MessageField2)
				start_animation();
			break;
		}
	case 36:
		{
			auto index = static_cast<int>(floor(value));
			if (index >= count || index < 0)
				break;

			auto light = List.at(index);
			light->Message(0, 0.0);
			if (MessageField2)
				start_animation();
			break;
		}
	case 37:
		{
			auto bmp1Count = 0;
			for (auto light : List)
			{
				if (light->BmpIndex1)
					++bmp1Count;
			}
			return bmp1Count;
		}
	case 38:
		return count;
	case 39:
		return MessageField2;
	case 40:
		return AnimationFlag;
	case 41:
		{
			auto index = next_light_up();
			if (index < 0)
				break;
			if (MessageField2 || AnimationFlag)
				TLightGroup::Message(34, 0.0);
			List.at(index)->Message(15, value);
			return 1;
		}
	case 42:
		{
			auto index = next_light_down();
			if (index < 0)
				break;
			if (MessageField2 || AnimationFlag)
				TLightGroup::Message(34, 0.0);
			List.at(index)->Message(16, value);
			return 1;
		}
	case 43:
		if (NotifyTimer)
			timer::kill(NotifyTimer);
		NotifyTimer = 0;
		if (value > 0.0f)
			NotifyTimer = timer::set(value, this, NotifyTimerExpired);
		break;
	case 44:
		{
			for (auto it = List.rbegin(); it != List.rend(); ++it)
			{
				auto light = *it;
				if (light->BmpIndex1)
				{
					light->Message(0, 0.0);
					light->Message(16, value);
				}
			}

			break;
		}
	case 45:
		{
			control::handler(code, this);
			auto index = static_cast<int>(floor(value));
			if (index >= 0 && index < count)
			{
				// Turn off lights (index, end]
				for (auto i = count - 1; i > index; i--)
				{
					List.at(i)->Message(20, 0.0);
				}

				// Turn on lights [begin, index]
				for (auto i = index; i >= 0; i--)
				{
					List.at(i)->Message(19, 0.0);
				}
			}
			break;
		}
	case 46:
		{
			auto index = next_light_down();
			if (index >= 0)
			{
				List.at(index)->Message(4, 0.0);
			}
			break;
		}
	default:
		for (auto it = List.rbegin(); it != List.rend(); ++it)
		{
			(*it)->Message(code, value);
		}
		break;
	}
	return 0;
}

void TLightGroup::Reset()
{
	if (Timer)
		timer::kill(Timer);
	Timer = 0;
	if (NotifyTimer)
		timer::kill(NotifyTimer);
	NotifyTimer = 0;
	MessageField2 = 0;
	AnimationFlag = 0;
	Timer1Time = Timer1TimeDefault;
}

void TLightGroup::reschedule_animation(float time)
{
	if (Timer)
		timer::kill(Timer);
	Timer = 0;
	if (time == 0)
	{
		MessageField2 = 0;
		AnimationFlag = 0;
		return;
	}

	Timer1Time = time > 0.0f ? time : Timer1TimeDefault;
	Timer = timer::set(Timer1Time, this, TimerExpired);
}

void TLightGroup::start_animation()
{
	for (auto it = List.rbegin(); it != List.rend(); ++it)
	{
		auto light = *it;
		if (light->BmpIndex1)
			light->Message(9, 0.0);
		else
			light->Message(8, 0.0);
	}
}

int TLightGroup::next_light_up()
{
	for (auto index = 0u; index < List.size(); ++index)
	{
		if (!List[index]->BmpIndex1)
			return static_cast<int>(index);
	}
	return -1;
}

int TLightGroup::next_light_down()
{
	for (auto index = static_cast<int>(List.size()) - 1; index >= 0; --index)
	{
		if (List.at(index)->BmpIndex1)
			return index;
	}
	return -1;
}

void TLightGroup::TimerExpired(int timerId, void* caller)
{
	auto group = static_cast<TLightGroup*>(caller);
	group->Timer = 0;
	group->Message(group->MessageField2, group->Timer1Time);
}

void TLightGroup::NotifyTimerExpired(int timerId, void* caller)
{
	auto group = static_cast<TLightGroup*>(caller);
	group->NotifyTimer = 0;
	control::handler(61, group);
}
