#pragma once
#include "objlist_class.h"
#include "TPinballComponent.h"

class TCollisionComponent : public TPinballComponent
{
public:
	objlist_class* EdgeList;
	__int16 UnknownC2;
	__int16 UnknownC3;
	float UnknownC4F;
	float UnknownC5F;
	float UnknownC6F;
	float UnknownC7F;
	int SoundIndex2;
	int SoundIndex1;

	TCollisionComponent(TPinballTable* table, int groupIndex, int someFlag);
	~TCollisionComponent();
};
