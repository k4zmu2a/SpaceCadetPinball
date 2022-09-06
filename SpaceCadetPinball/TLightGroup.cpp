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

int TLightGroup::Message(MessageCode code, float value)
{
	auto const count = static_cast<int>(List.size());
	switch (code)
	{
	case MessageCode::SetTiltLock:
	case MessageCode::GameOver:
		break;
	case MessageCode::PlayerChanged:
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
	case MessageCode::Reset:
		Reset();
		for (auto index = 0; index < PinballTable->PlayerCount; index++)
		{
			auto playerPtr = &PlayerData[index];
			playerPtr->MessageField = MessageField;
			playerPtr->MessageField2 = MessageField2;
			playerPtr->Timer1Time = Timer1Time;
		}
		break;
	case MessageCode::TLightGroupStepBackward:
		{
			auto lastLight = List.at(count - 1);
			if (lastLight->FlasherOnFlag || lastLight->ToggledOnFlag || lastLight->ToggledOffFlag)
				break;
			if (MessageField2 != MessageCode::TLightGroupNull)
			{
				TLightGroup::Message(MessageCode::TLightGroupReset, 0.0);
			}
			AnimationFlag = 1;
			MessageField2 = code;
			auto lastMessage = lastLight->MessageField;
			auto lastStatus = lastLight->LightOnFlag;
			for (auto index = count - 1; index > 0; --index)
			{
				auto lightCur = List.at(index);
				auto lightPrev = List.at(index - 1);
				lightCur->Message(lightPrev->LightOnFlag ? MessageCode::TLightTurnOn : MessageCode::TLightTurnOff, 0.0);
				lightCur->MessageField = lightPrev->MessageField;
			}
			auto firstLight = List.at(0);
			firstLight->Message(lastStatus ? MessageCode::TLightTurnOn : MessageCode::TLightTurnOff, 0.0);
			firstLight->MessageField = lastMessage;
			reschedule_animation(value);
			break;
		}
	case MessageCode::TLightGroupStepForward:
		{
			auto lastLight = List.at(count - 1);
			if (lastLight->FlasherOnFlag || lastLight->ToggledOnFlag || lastLight->ToggledOffFlag)
				break;
			if (MessageField2 != MessageCode::TLightGroupNull)
			{
				TLightGroup::Message(MessageCode::TLightGroupReset, 0.0);
			}
			auto firstLight = List.at(0);
			AnimationFlag = 1;
			MessageField2 = code;
			auto firstMessage = firstLight->MessageField;
			auto firstStatus = firstLight->LightOnFlag;
			for (auto index = 0; index < count - 1; index++)
			{
				auto lightCur = List.at(index);
				auto lightNext = List.at(index + 1);
				lightCur->Message(lightNext->LightOnFlag ? MessageCode::TLightTurnOn : MessageCode::TLightTurnOff, 0.0);
				lightCur->MessageField = lightNext->MessageField;
			}
			lastLight->Message(firstStatus ? MessageCode::TLightTurnOn : MessageCode::TLightTurnOff, 0.0);
			lastLight->MessageField = firstMessage;
			reschedule_animation(value);
			break;
		}
	case MessageCode::TLightGroupAnimationBackward:
		{
			if (AnimationFlag || MessageField2 == MessageCode::TLightGroupNull)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			auto lastLight = List.at(count - 1);
			auto lastStatus = lastLight->ToggledOnFlag;
			for (auto i = count - 1; i > 0; --i)
			{
				auto lightCur = List.at(i);
				auto lightPrev = List.at(i - 1);
				lightCur->Message(lightPrev->ToggledOnFlag ? MessageCode::TLightTurnOnTimed : MessageCode::TLightTurnOffTimed, 0.0);
			}
			auto firstLight = List.at(0);
			firstLight->Message(lastStatus ? MessageCode::TLightTurnOnTimed : MessageCode::TLightTurnOffTimed, 0);
			reschedule_animation(value);
			break;
		}
	case MessageCode::TLightGroupAnimationForward:
		{
			if (AnimationFlag || MessageField2 == MessageCode::TLightGroupNull)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			auto firstLight = List.at(0);
			auto firstStatus = firstLight->ToggledOnFlag;
			for (auto i = 0; i < count - 1; i++)
			{
				auto lightCur = List.at(i);
				auto lightNext = List.at(i + 1);
				lightCur->Message(lightNext->ToggledOnFlag ? MessageCode::TLightTurnOnTimed : MessageCode::TLightTurnOffTimed, 0.0);
			}
			auto lastLight = List.at(count - 1);
			lastLight->Message(firstStatus ? MessageCode::TLightTurnOnTimed : MessageCode::TLightTurnOffTimed, 0);
			reschedule_animation(value);
			break;
		}
	case MessageCode::TLightGroupLightShowAnimation:
		{
			if (AnimationFlag || MessageField2 == MessageCode::TLightGroupNull)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			for (auto light : List)
			{
				if (rand() % 100 > 70)
				{
					auto randVal = RandFloat() * value * 3.0f + 0.1f;
					light->Message(MessageCode::TLightTurnOnTimed, randVal);
				}
			}
			reschedule_animation(value);
			break;
		}
	case MessageCode::TLightGroupGameOverAnimation:
		{
			if (AnimationFlag || MessageField2 == MessageCode::TLightGroupNull)
				start_animation();
			MessageField2 = code;
			AnimationFlag = 0;
			for (auto light : List)
			{
				auto randVal = static_cast<float>(rand() % 100 > 70);
				light->Message(MessageCode::TLightResetAndToggleValue, randVal);
			}
			reschedule_animation(value);
			break;
		}
	case MessageCode::TLightGroupRandomAnimationSaturation:
		{
			auto noBmpInd1Count = 0;
			for (auto light : List)
			{
				if (!light->LightOnFlag)
					++noBmpInd1Count;
			}
			if (!noBmpInd1Count)
				break;

			auto randModCount = rand() % noBmpInd1Count;
			for (auto it = List.rbegin(); it != List.rend(); ++it)
			{
				auto light = *it;
				if (!light->LightOnFlag && randModCount-- == 0)
				{
					light->Message(MessageCode::TLightTurnOn, 0.0);
					break;
				}
			}

			if (MessageField2 != MessageCode::TLightGroupNull)
				start_animation();
			break;
		}
	case MessageCode::TLightGroupRandomAnimationDesaturation:
		{
			auto bmpInd1Count = 0;
			for (auto light : List)
			{
				if (light->LightOnFlag)
					++bmpInd1Count;
			}
			if (!bmpInd1Count)
				break;

			auto randModCount = rand() % bmpInd1Count;
			for (auto it = List.rbegin(); it != List.rend(); ++it)
			{
				auto light = *it;
				if (light->LightOnFlag && randModCount-- == 0)
				{
					light->Message(MessageCode::TLightTurnOff, 0.0);
					break;
				}
			}

			if (MessageField2 != MessageCode::TLightGroupNull)
				start_animation();
			break;
		}
	case MessageCode::TLightGroupOffsetAnimationForward:
		{
			auto index = next_light_up();
			if (index < 0)
				break;
			List.at(index)->Message(MessageCode::TLightTurnOn, 0.0);
			if (MessageField2 != MessageCode::TLightGroupNull)
				start_animation();
			return 1;
		}
	case MessageCode::TLightGroupOffsetAnimationBackward:
		{
			auto index = next_light_down();
			if (index < 0)
				break;
			List.at(index)->Message(MessageCode::TLightTurnOff, 0.0);
			if (MessageField2 != MessageCode::TLightGroupNull)
				start_animation();
			return 1;
		}
	case MessageCode::TLightGroupReset:
		{
			if (Timer)
				timer::kill(Timer);
			Timer = 0;
			if (MessageField2 == MessageCode::TLightGroupAnimationBackward ||
				MessageField2 == MessageCode::TLightGroupAnimationForward || MessageField2 == MessageCode::TLightGroupLightShowAnimation)
				TLightGroup::Message(MessageCode::TLightResetTimed, 0.0);
			MessageField2 = MessageCode::TLightGroupNull;
			AnimationFlag = 0;
			break;
		}
	case MessageCode::TLightGroupTurnOnAtIndex:
		{
			auto index = static_cast<int>(floor(value));
			if (index >= count || index < 0)
				break;

			auto light = List.at(index);
			light->Message(MessageCode::TLightTurnOn, 0.0);
			if (MessageField2 != MessageCode::TLightGroupNull)
				start_animation();
			break;
		}
	case MessageCode::TLightGroupTurnOffAtIndex:
		{
			auto index = static_cast<int>(floor(value));
			if (index >= count || index < 0)
				break;

			auto light = List.at(index);
			light->Message(MessageCode::TLightTurnOff, 0.0);
			if (MessageField2 != MessageCode::TLightGroupNull)
				start_animation();
			break;
		}
	case MessageCode::TLightGroupGetOnCount:
		{
			auto bmp1Count = 0;
			for (auto light : List)
			{
				if (light->LightOnFlag)
					++bmp1Count;
			}
			return bmp1Count;
		}
	case MessageCode::TLightGroupGetLightCount:
		return count;
	case MessageCode::TLightGroupGetMessage2:
		return ~MessageField2;
	case MessageCode::TLightGroupGetAnimationFlag:
		return AnimationFlag;
	case MessageCode::TLightGroupResetAndTurnOn:
		{
			auto index = next_light_up();
			if (index < 0)
				break;
			if (MessageField2 != MessageCode::TLightGroupNull || AnimationFlag)
				TLightGroup::Message(MessageCode::TLightGroupReset, 0.0);
			List.at(index)->Message(MessageCode::TLightFlasherStartTimedThenStayOn, value);
			return 1;
		}
	case MessageCode::TLightGroupResetAndTurnOff:
		{
			auto index = next_light_down();
			if (index < 0)
				break;
			if (MessageField2 != MessageCode::TLightGroupNull || AnimationFlag)
				TLightGroup::Message(MessageCode::TLightGroupReset, 0.0);
			List.at(index)->Message(MessageCode::TLightFlasherStartTimedThenStayOff, value);
			return 1;
		}
	case MessageCode::TLightGroupRestartNotifyTimer:
		if (NotifyTimer)
			timer::kill(NotifyTimer);
		NotifyTimer = 0;
		if (value > 0.0f)
			NotifyTimer = timer::set(value, this, NotifyTimerExpired);
		break;
	case MessageCode::TLightGroupFlashWhenOn:
		{
			for (auto it = List.rbegin(); it != List.rend(); ++it)
			{
				auto light = *it;
				if (light->LightOnFlag)
				{
					light->Message(MessageCode::TLightTurnOff, 0.0);
					light->Message(MessageCode::TLightFlasherStartTimedThenStayOff, value);
				}
			}

			break;
		}
	case MessageCode::TLightGroupToggleSplitIndex:
		{
			control::handler(~code, this);
			auto index = static_cast<int>(floor(value));
			if (index >= 0 && index < count)
			{
				// Turn off lights (index, end]
				for (auto i = count - 1; i > index; i--)
				{
					List.at(i)->Message(MessageCode::TLightResetAndTurnOff, 0.0);
				}

				// Turn on lights [begin, index]
				for (auto i = index; i >= 0; i--)
				{
					List.at(i)->Message(MessageCode::TLightResetAndTurnOn, 0.0);
				}
			}
			break;
		}
	case MessageCode::TLightGroupStartFlasher:
		{
			auto index = next_light_down();
			if (index >= 0)
			{
				List.at(index)->Message(MessageCode::TLightFlasherStart, 0.0);
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
	MessageField2 = MessageCode::TLightGroupNull;
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
		MessageField2 = MessageCode::TLightGroupNull;
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
		if (light->LightOnFlag)
			light->Message(MessageCode::TLightTurnOnTimed, 0.0);
		else
			light->Message(MessageCode::TLightTurnOffTimed, 0.0);
	}
}

int TLightGroup::next_light_up()
{
	for (auto index = 0u; index < List.size(); ++index)
	{
		if (!List[index]->LightOnFlag)
			return static_cast<int>(index);
	}
	return -1;
}

int TLightGroup::next_light_down()
{
	for (auto index = static_cast<int>(List.size()) - 1; index >= 0; --index)
	{
		if (List.at(index)->LightOnFlag)
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
