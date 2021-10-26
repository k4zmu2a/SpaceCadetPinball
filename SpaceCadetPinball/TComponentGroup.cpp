#include "pch.h"
#include "TComponentGroup.h"


#include "control.h"
#include "loader.h"
#include "timer.h"
#include "TPinballTable.h"

TComponentGroup::TComponentGroup(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, false)
{
	Timer = 0;
	if (groupIndex > 0)
	{
		int attrCount;
		auto shortArr = loader::query_iattribute(groupIndex, 1027, &attrCount);
		auto shortArrPtr = shortArr;
		for (auto index = 0; index < attrCount; ++index, ++shortArrPtr)
		{
			auto component = table->find_component(*shortArrPtr);
			if (component)
				List.push_back(component);
		}
	}
}

TComponentGroup::~TComponentGroup()
{
	if (Timer)
	{
		timer::kill(Timer);
		Timer = 0;
	}
}

int TComponentGroup::Message(int code, float value)
{
	if (code == 48)
	{
		if (this->Timer)
		{
			timer::kill(this->Timer);
			this->Timer = 0;
		}
		if (value > 0.0f)
			this->Timer = timer::set(value, this, NotifyTimerExpired);
	}
	else if (code <= 1007 || (code > 1011 && code != 1020 && code != 1022))
	{
		for (auto component : List)
		{
			component->Message(code, value);
		}
	}
	return 0;
}

void TComponentGroup::NotifyTimerExpired(int timerId, void* caller)
{
	auto compGroup = static_cast<TComponentGroup*>(caller);
	compGroup->Timer = 0;
	control::handler(61, compGroup);
}
