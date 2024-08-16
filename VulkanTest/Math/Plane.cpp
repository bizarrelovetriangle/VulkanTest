#include "Plane.h"
#include "../CAD/MeshModel.h";

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

std::vector<Vector3f> Plane::MeshIntersections(const MeshModel& mesh) const
{
	std::vector<Vector3f> result;
	
	for (uint32_t tri = 0; tri < mesh.triangleBitVector.size(); ++tri)
	{
		if (!mesh.triangleBitVector[tri])
		{
			continue;
		}

		Vector3f intersection;
		auto& triangle = mesh.triangles[tri];

		for (int i = 0; i < 3; ++i)
		{
			auto& org = triangle.vertices[i];
			auto& dest = triangle.vertices[(i + 1) % 3];

			if (org < dest) {
				if (Intersect(mesh.points[org], mesh.points[dest], &intersection)) {
					result.push_back(intersection);
				}
			}
		}
	}
	return result;
}

bool Plane::Intersect(const Vector3f& segmentA, const Vector3f& segmentB, Vector3f* intersectPoint, float* ratio) const
{
	float a = Distance(segmentA);
	float b = Distance(segmentB);

	if (a * b > 0.)
		return false;

	float r = a / (a - b);
	if (ratio) {
		*ratio = r;
	}

	if (intersectPoint) {
		*intersectPoint = segmentB * r + segmentA * (1 - r);
	}
	return true;
}

Matrix4 Plane::getMatrix() const
{
	return Matrix4::Translate(normal * dist) * Matrix4::Rotate(Vector3f(0., 1., 0.), normal);
}

