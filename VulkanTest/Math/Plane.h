#pragma once
#include "Vector3.hpp";
#include "Matrix4.hpp";

class Plane
{
public:
	Plane(const Vector3f& normal, float dist);
	Plane(const Vector3f& pos, const Vector3f& normal);
	static Plane fromTwoPoints(const Vector3f& pos, const Vector3f& dest);
	Matrix4 getMatrix();

	Vector3f normal;
	float dist;
};