#pragma once

class TBall;
class TCollisionComponent;
struct ray_type;

enum class wall_type : int
{
	Circle = 0,
	Line = 1,
};

class TEdgeSegment
{
public:
	TCollisionComponent* CollisionComponent;
	char* PinbCompFlag2Ptr;
	char ProcessedFlag;
	int WallValue;
	int VisualFlag;

	TEdgeSegment(TCollisionComponent* collComp, char* someFlag, unsigned int visualFlag);
	virtual ~TEdgeSegment() = default;

	virtual void EdgeCollision(TBall* ball, float coef) = 0;
	virtual void port_draw();
	virtual void place_in_grid() = 0;
	virtual float FindCollisionDistance(ray_type* ray) = 0;

	static TEdgeSegment* install_wall(float* floatArr, TCollisionComponent* collComp, char* flagPtr,
	                                  unsigned int visual_flag, float offset, int someValue);
};
