#include "pch.h"
#include "TCircle.h"

#include "TBall.h"
#include "TCollisionComponent.h"
#include "TTableLayer.h"

TCircle::TCircle(TCollisionComponent* collComp, char* activeFlag, unsigned collisionGroup, vector2* center,
                 float radius): TEdgeSegment(collComp, activeFlag, collisionGroup)
{
	Circle.RadiusSq = radius * radius;
	Circle.Center = *center;
}

float TCircle::FindCollisionDistance(const ray_type& ray)
{
	return maths::ray_intersect_circle(ray, Circle);
}

void TCircle::EdgeCollision(TBall* ball, float distance)
{
	vector2 direction{}, nextPosition{};

	nextPosition.X = distance * ball->Direction.X + ball->Position.X;
	nextPosition.Y = distance * ball->Direction.Y + ball->Position.Y;
	direction.X = nextPosition.X - Circle.Center.X;
	direction.Y = nextPosition.Y - Circle.Center.Y;
	maths::normalize_2d(direction);
	CollisionComponent->Collision(ball, &nextPosition, &direction, distance, this);
}

void TCircle::place_in_grid(RectF* aabb)
{
	if(aabb)
	{
		const auto radius = sqrt(Circle.RadiusSq);
		aabb->Merge({
			Circle.Center.X + radius, Circle.Center.Y + radius,
			Circle.Center.X - radius, Circle.Center.Y - radius
		});
	}

	TTableLayer::edges_insert_circle(&Circle, this, nullptr);
}
