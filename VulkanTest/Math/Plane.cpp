#include "Plane.h"

Plane::Plane() : normal(Vector3f(0., 0., 1.)), dist(0.)
{
}

Plane::Plane(const Vector3f& normal, float dist)
	: normal(normal), dist(dist)
{
}

Plane::Plane(const Vector3f& pos, const Vector3f& normal)
{
	this->normal = normal;
	dist = pos.Dot(normal);
}

Plane Plane::fromTwoPoints(const Vector3f& pos, const Vector3f& dest)
{
	return Plane(pos, (dest - pos).Normalized());
}

float Plane::Distance(const Vector3f& point) const
{
	return normal.Dot(point) - dist;
}

bool Plane::Intersect(const Vector3f& segmentA, const Vector3f& segmentB, Vector3f* intersectPoint, float* ratio) const
{
	float a = Distance(segmentA);
	float b = Distance(segmentB);

	if (a * b > 0.)
		return false;

	float r = a / (a - b);
	if (ratio)
		*ratio = r;

	if (intersectPoint)
		*intersectPoint = segmentB * r + segmentA * (1 - r);
}

Matrix4 Plane::getMatrix() const
{
	auto pos = normal * dist;
	auto j = normal;
	auto [i, k] = j.twoPerpendiculars();

	Matrix4 asixRotate{
		{i.x  ,   i.y,   i.z, 0.},
		{j.x  ,   j.y,   j.z, 0.},
		{k.x  ,   k.y,   k.z, 0.},
		{pos.x, pos.y, pos.z, 1.} };

	return asixRotate;
}

