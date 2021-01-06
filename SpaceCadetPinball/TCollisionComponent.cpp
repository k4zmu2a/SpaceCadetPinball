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
	delete EdgeList;
}


void TCollisionComponent::port_draw()
{
	for (int index = EdgeList->Count() - 1; index >= 0; index--)
	{
		static_cast<TEdgeSegment*>(EdgeList->Get(index))->port_draw();
	}
}

int TCollisionComponent::DefaultCollision(TBall* ball, vector_type* ballPosition, vector_type* vec2)
{
	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, ballPosition, vec2, UnknownC4F, UnknownC5F, 1000000000.0, 0.0);
		return 0;
	}
	auto projSpeed = maths::basic_collision(ball, ballPosition, vec2, UnknownC4F, UnknownC5F,
	                                        UnknownC7F,
	                                        UnknownC6F);
	if (projSpeed <= UnknownC7F)
	{
		if (projSpeed > 0.2)
		{
			if (SoundIndex2)
				loader::play_sound(SoundIndex2);
		}
		return 0;
	}
	if (SoundIndex1)
		loader::play_sound(SoundIndex1);
	return 1;
}

void TCollisionComponent::Collision(TBall* ball, struct vector_type* ballPosition, struct vector_type* vec2,
                                    float someVal, TEdgeSegment* edge)
{
	int soundIndex;

	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, ballPosition, vec2, UnknownC4F, UnknownC5F, 1000000000.0, 0.0);
		return;
	}
	double projSpeed = maths::basic_collision(
		ball,
		ballPosition,
		vec2,
		UnknownC4F,
		UnknownC5F,
		UnknownC7F,
		UnknownC6F);
	if (projSpeed <= UnknownC7F)
	{
		if (projSpeed <= 0.2)
			return;
		soundIndex = SoundIndex2;
	}
	else
	{
		soundIndex = SoundIndex1;
	}
	if (soundIndex)
		loader::play_sound(soundIndex);
}

int TCollisionComponent::FieldEffect(TBall* ball, vector_type* vecDst)
{
	return 0;
}
