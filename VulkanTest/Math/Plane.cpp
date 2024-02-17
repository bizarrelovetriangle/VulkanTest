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

Matrix4 Plane::getMatrix()
{
	auto pos = normal * dist;
	auto j = normal;
	auto [i, k] = j.twoPerpendicularsForJ();

	Matrix4 asixRotate{
		{i.x, j.x, k.x, pos.x},
		{i.y, j.y, k.y, pos.y},
		{i.z, j.z, k.z, pos.z},
		{ 0.,  0.,  0.,    1.}};

	return asixRotate;
}

