#include "pch.h"
#include "TTripwire.h"

#include "control.h"
#include "loader.h"
#include "TBall.h"
#include "TPinballTable.h"

TTripwire::TTripwire(TPinballTable* table, int groupIndex) : TRollover(table, groupIndex, true)
{
}

void TTripwire::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
                          TEdgeSegment* edge)
{
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance -= distance;
	ball->not_again(edge);
	if (!PinballTable->TiltLockFlag)
	{
		loader::play_sound(SoftHitSoundId, ball, "TTripwire");
		control::handler(MessageCode::ControlCollision, this);
	}
}
