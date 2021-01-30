#pragma once
#include "TPinballComponent.h"


class TComponentGroup :
	public TPinballComponent
{
public:
	TComponentGroup(TPinballTable* table, int groupIndex);
	~TComponentGroup() override;
	int Message(int code, float value) override;
	static void NotifyTimerExpired(int timerId, void* caller);

	objlist_class<TPinballComponent>* List;
	int Timer;
};
