#pragma once
#include "../Math/Vector3.h"
#include "../Math/Plane.h"
#include "MeshModel.h"
#include <map>
#include <set>

class GeometryFunctions
{
public:
	static bool SegmentTriangleIntersetion(const std::pair<Vector3f, Vector3f>& line,
		const Vector3f& triangleA, const Vector3f& triangleB, const Vector3f& triangleC,
		Vector3f& intersectPoint, float* dist = nullptr)
	{
		auto triNorm = (triangleB - triangleA).Cross(triangleC - triangleA).Normalized();

		if (triNorm.Dot(line.second - line.first) > 0.)
			return false;

		Plane plane(triangleA, triNorm);
		if (plane.Intersect(line.first, line.second, &intersectPoint))
		{
			Vector3f a_b_point_vector = (triangleB - triangleA).Cross(intersectPoint - triangleA);
			Vector3f b_c_point_vector = (triangleC - triangleB).Cross(intersectPoint - triangleB);
			Vector3f c_a_point_vector = (triangleA - triangleC).Cross(intersectPoint - triangleC);

			bool inside =
				triNorm.Dot(a_b_point_vector) > 0 &&
				triNorm.Dot(b_c_point_vector) > 0 &&
				triNorm.Dot(c_a_point_vector) > 0;

			if (inside && dist) {
				*dist = (intersectPoint - line.first).Length();
			}

			return inside;
		}
	}
	
	static bool PointInsideTriangularPyramid(const Vector3f& point, const MeshModel& mesh, const std::vector<uint32_t> triIndexes)
	{
		auto& meshPoints = mesh.points;
		for (uint32_t triIndex : triIndexes)
		{
			auto& triVerts = mesh.triangles[triIndex].vertices;
			float dist = TrianglePlanePointDist(
				meshPoints[triVerts[0]], meshPoints[triVerts[1]], meshPoints[triVerts[2]], point);
			if (dist > 0) return false;
		}
		return true;
	}

	static float TrianglePlanePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		return normal.Dot(point - a);
	}

	static float TrianglePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		// here we check if the point is inside a prism that created with sides and normal of the triangle
		// if the point is outside two sides simultaniously, calculating the distance for only one of them will be sufficient
		auto triNormal = (b - a).Cross(c - a);
		Vector3f a_b_point_vector = (b - a).Cross(point - a);
		Vector3f b_c_point_vector = (c - b).Cross(point - b);
		Vector3f c_a_point_vector = (a - c).Cross(point - c);

		auto test1 = triNormal.Dot(a_b_point_vector) > 0;
		auto test2 = triNormal.Dot(b_c_point_vector) > 0;
		auto test3 = triNormal.Dot(c_a_point_vector) > 0;

		if (!test1) return LinePointDist(a, b, point);
		if (!test2) return LinePointDist(b, c, point);
		if (!test3) return LinePointDist(c, a, point);

		auto normal = (b - a).Cross(c - a).Normalized();
		float planeDist = std::abs(normal.Dot(point - a));
		return planeDist;
	}

	static float LinePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& point)
	{
		auto a_b = b - a;
		auto a_b_normalized = a_b.Normalized();
		auto proj_length = a_b_normalized.Dot(point - a);
		if (proj_length < 0.) return (point - a).Length();
		if (proj_length > a_b.Length()) return (point - b).Length();
		return (a_b_normalized * proj_length + a - point).Length();
	}

	static Vector3f TrianglePointDir(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		float planeDist = normal.Dot(point - a);
		return (normal * planeDist).Normalized();
	}

	static bool ContourIsCycled(const std::set<std::pair<uint32_t, uint32_t>>& contour)
	{
		if (contour.size() == 0) {
			return true;
		}

		std::map<uint32_t, uint32_t> map;
		for (auto& edge : contour) {
			if (auto p = map.emplace(edge.first, edge.second); !p.second) {
				return false;
			}
		}

		uint32_t initial = map.begin()->first;
		auto vert = initial;

		for (int i = 0; i < contour.size(); ++i) {
			if (auto it = map.find(vert); it != map.end()) {
				vert = it->second;
				map.erase(it);
			}
			else {
				return false;
			}
		}

		if (vert != initial) {
			return false;
		}

		return true;
	}
};
