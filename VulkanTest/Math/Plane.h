#pragma once
#include "Vector3.h";
#include "Matrix4.h";

class Plane
{
public:
	Plane();
	Plane(const Vector3f& normal, float dist);
	Plane(const Vector3f& pos, const Vector3f& normal);
	static Plane fromTwoPoints(const Vector3f& pos, const Vector3f& dest);
	Matrix4 getMatrix() const;

	Vector3f normal;
	float dist;
};