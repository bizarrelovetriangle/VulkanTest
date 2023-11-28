#include "Plane.h"

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

Matrix4 Plane::getMatrix()
{
	auto pos = normal * dist;
	auto [i, k] = normal.twoPerpendicularsForJ();
	i = Vector3f(0.,0.,-1.);
	auto mat = Matrix4::LookAt(pos, pos + i);
	return Matrix4::LookAt(pos, pos + i);
}

