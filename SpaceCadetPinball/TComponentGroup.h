#pragma once
#include "TPinballComponent.h"


class TComponentGroup :
	public TPinballComponent2
{
public:
	TComponentGroup(TPinballTable* table, int groupIndex);
	~TComponentGroup() override;
	int Message2(MessageCode code, float value) override;
	static void NotifyTimerExpired(int timerId, void* caller);

	std::vector<TPinballComponent*> List;
	int Timer;
};
