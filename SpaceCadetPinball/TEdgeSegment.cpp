#include "pch.h"
#include "TEdgeSegment.h"

TEdgeSegment::TEdgeSegment(TCollisionComponent* collComp, char* someFlag, unsigned visualFlag)
{
	this->CollisionComponent = collComp;
	this->PinbCompFlag2Ptr = someFlag;
	this->VisualFlag = visualFlag;
	this->Unknown3_0 = 0;
}
