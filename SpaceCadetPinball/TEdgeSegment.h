#pragma once

class TBall;
class TCollisionComponent;
struct ray_type;
struct RectF;

enum class wall_type : int
{
	Circle = 0,
	Line = 1,
};

class TEdgeSegment
{
public:
	TCollisionComponent* CollisionComponent;
	char* ActiveFlagPtr;
	char ProcessedFlag;
	void* WallValue{};
	unsigned int CollisionGroup;

	TEdgeSegment(TCollisionComponent* collComp, char* activeFlag, unsigned int collisionGroup);
	virtual ~TEdgeSegment() = default;

	virtual void EdgeCollision(TBall* ball, float distance) = 0;
	virtual void port_draw();
	virtual void place_in_grid(RectF* aabb) = 0;
	virtual float FindCollisionDistance(const ray_type& ray) = 0;

	static TEdgeSegment* install_wall(float* floatArr, TCollisionComponent* collComp, char* activeFlagPtr,
	                                  unsigned int collisionGroup, float offset, size_t wallValue);
};
