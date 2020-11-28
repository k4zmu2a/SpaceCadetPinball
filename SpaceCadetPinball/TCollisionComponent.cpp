#include "pch.h"
#include "TCollisionComponent.h"
#include "loader.h"
#include "TEdgeSegment.h"
#include "TPinballTable.h"


TCollisionComponent::TCollisionComponent(TPinballTable* table, int groupIndex, bool createWall) : TPinballComponent(
	table, groupIndex, true)
{
	visualStruct visual{};

	EdgeList = new objlist_class(4, 4);
	UnknownBaseFlag2 = 1;
	if (GroupName != nullptr)
		UnknownBaseFlag1 = 1;
	if (groupIndex <= 0)
	{
		loader::default_vsi(&visual);
	}
	else
	{
		loader::query_visual(groupIndex, 0, &visual);
		if (createWall)
		{
			float offset = table->CollisionCompOffset;
			float* floatArr = loader::query_float_attribute(groupIndex, 0, 600);
			TEdgeSegment::install_wall(floatArr, this, &UnknownBaseFlag2, visual.Flag, offset, 0);
		}
	}

	UnknownC7F = visual.Kicker.Unknown1F;
	UnknownC4F = visual.Unknown2F;
	UnknownC5F = visual.Unknown1F;
	UnknownC6F = visual.Kicker.Unknown2F;
	SoundIndex1 = visual.Kicker.SoundIndex;
	SoundIndex2 = visual.SoundIndex2;
	GroupIndex = groupIndex;
}

TCollisionComponent::~TCollisionComponent()
{
	for (TEdgeSegment* edge; EdgeList->Count() > 0;)
	{
		edge = static_cast<TEdgeSegment*>(EdgeList->Get(0));
		EdgeList->Delete(edge);
		delete edge;
	}
	delete this->EdgeList;
}
