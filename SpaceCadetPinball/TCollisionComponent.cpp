#include "pch.h"
#include "TCollisionComponent.h"
#include "loader.h"
#include "maths.h"
#include "objlist_class.h"
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

	Threshold = visual.Kicker.Threshold;
	Elasticity = visual.Elasticity;
	Smoothness = visual.Smoothness;
	Boost = visual.Kicker.Boost;
	HardHitSoundId = visual.Kicker.HardHitSoundId;
	SoftHitSoundId = visual.SoftHitSoundId;
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
	delete EdgeList;
}


void TCollisionComponent::port_draw()
{
	for (int index = EdgeList->Count() - 1; index >= 0; index--)
	{
		static_cast<TEdgeSegment*>(EdgeList->Get(index))->port_draw();
	}
}

int TCollisionComponent::DefaultCollision(TBall* ball, vector_type* nextPosition, vector_type* direction)
{
	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 1000000000.0, 0.0);
		return 0;
	}
	auto projSpeed = maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, Threshold, Boost);
	if (projSpeed <= Threshold)
	{
		if (projSpeed > 0.2)
		{
			if (SoftHitSoundId)
				loader::play_sound(SoftHitSoundId);
		}
		return 0;
	}
	if (HardHitSoundId)
		loader::play_sound(HardHitSoundId);
	return 1;
}

void TCollisionComponent::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction,
                                    float coef, TEdgeSegment* edge)
{
	int soundIndex;

	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 1000000000.0, 0.0);
		return;
	}
	double projSpeed = maths::basic_collision(
		ball,
		nextPosition,
		direction,
		Elasticity,
		Smoothness,
		Threshold,
		Boost);
	if (projSpeed <= Threshold)
	{
		if (projSpeed <= 0.2)
			return;
		soundIndex = SoftHitSoundId;
	}
	else
	{
		soundIndex = HardHitSoundId;
	}
	if (soundIndex)
		loader::play_sound(soundIndex);
}

int TCollisionComponent::FieldEffect(TBall* ball, vector_type* vecDst)
{
	return 0;
}
