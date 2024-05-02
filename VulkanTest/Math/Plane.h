#pragma once
#include "Vector3.h";
#include "Matrix4.h";

class MeshModel;

class Plane
{
public:
	Plane();
	Plane(const Vector3f& normal, float dist);
	Plane(const Vector3f& pos, const Vector3f& normal);
	static Plane fromTwoPoints(const Vector3f& pos, const Vector3f& dest);
	float Distance(const Vector3f& point) const;
	bool Intersect(const Vector3f& segmentA, const Vector3f& segmentB, Vector3f* intersectPoint = nullptr, float* ratio = nullptr) const;
	std::vector<Vector3f> MeshIntersections(const MeshModel& mesh) const;
	Matrix4 getMatrix() const;


	Vector3f normal;
	float dist;
};