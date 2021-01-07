#include "pch.h"
#include "TCircle.h"

TCircle::TCircle(TCollisionComponent* collComp, char* someFlagPtr, unsigned visualFlag, vector_type* center,
                 float radius): TEdgeSegment(collComp, someFlagPtr, visualFlag)
{
	Circle.RadiusSq = radius * radius;
	Circle.Center = *center;
}

float TCircle::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_circle(ray, &Circle);
}

void TCircle::EdgeCollision(TBall* ball, float coef)
{
	vector_type direction{}, nextPosition{};

	nextPosition.X = coef * ball->Acceleration.X + ball->Position.X;
	nextPosition.Y = coef * ball->Acceleration.Y + ball->Position.Y;
	direction.X = nextPosition.X - Circle.Center.X;
	direction.Y = nextPosition.Y - Circle.Center.Y;
	maths::normalize_2d(&direction);
	CollisionComponent->Collision(ball, &nextPosition, &direction, coef, this);
}
