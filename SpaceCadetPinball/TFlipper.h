#pragma once
#include "TCollisionComponent.h"

class TFlipperEdge;

class TFlipper :
	public TCollisionComponent2
{
public:
	TFlipper(TPinballTable* table, int groupIndex);
	~TFlipper() override;
	int Message2(MessageCode code, float value) override;
	void port_draw() override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;
	void UpdateSprite(float timeNow);
	
	int BmpIndex;
	TFlipperEdge* FlipperEdge;
};
