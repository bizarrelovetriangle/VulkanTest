#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include "../Math/Vector3.h"

struct Edge
{
	size_t triangleIndex = 0;
	size_t side = 0;

	friend bool operator==(const Edge& a, const Edge& b) {
		return a.triangleIndex == b.triangleIndex;
	}
};

struct Triangle
{
	std::array<uint32_t, 3> vertices;
	std::array<Edge, 3> edges;
};

struct KeyHasher
{
	size_t operator()(const std::pair<uint32_t, uint32_t>& pair) const
	{
		return std::hash<uint32_t>{}(pair.first) ^ std::hash<uint32_t>{}(pair.second);
	}

	size_t operator()(const Edge& edge) const
	{
		return std::hash<uint32_t>{}(edge.triangleIndex) ^ std::hash<uint32_t>{}(edge.side);
	}
};

class MeshModel
{
public:
	MeshModel(const std::vector<uint32_t>& indexes, const std::vector<Vector3f>& points)
	{
		triangles.resize(indexes.size() / 3);
		this->points = points;

		for (size_t i = 0; i < indexes.size() / 3; ++i) {
			auto& triangle = triangles.at(i);

			for (int j = 0; j < 3; ++j) {
				triangle.vertices[j] = indexes.at(i * 3 + j);
				triangle.edges[j].triangleIndex = i;
				triangle.edges[j].side = j;
			}
		}

		std::unordered_map<std::pair<uint32_t, uint32_t>, Edge, KeyHasher> map;
		for (auto& triangle : triangles) {
			for (size_t i = 0; i < 3; ++i) {
				auto& edge = triangle.edges.at(i);
				uint32_t org = Origin(edge);
				uint32_t dest = Destination(edge);
				if (org > dest) std::swap(org, dest);

				if (auto it = map.emplace(std::make_pair(org, dest), edge); !it.second) {
					combinedEdges.emplace(edge, it.first->second);
					combinedEdges.emplace(it.first->second, edge);
				}
			}
		}
	}

	uint32_t Origin(const Edge& edge)
	{
		auto& triangle = triangles.at(edge.triangleIndex);
		return triangle.vertices.at(edge.side);
	}

	uint32_t Destination(const Edge& edge)
	{
		auto& triangle = triangles.at(edge.triangleIndex);
		return triangle.vertices.at((edge.side + 1) % 3);
	}

	Edge combinedTriangle(const Edge& edge)
	{
		return combinedEdges.at(edge);
	}

	std::array<Vector3f, 3> TrianglePoints(const Triangle& triangle) const
	{
		uint32_t a = triangle.vertices[0];
		uint32_t b = triangle.vertices[1];
		uint32_t c = triangle.vertices[2];
		return { points[a], points[b], points[c] };
	}

	Vector3f TriangleNormal(const Triangle& triangle) const
	{
		auto trianglePoints = TrianglePoints(triangle);
		return (trianglePoints[1] - trianglePoints[0]).Cross(trianglePoints[2] - trianglePoints[0]);
	}

//private:
	std::vector<Triangle> triangles;
	std::vector<Vector3f> points;
	std::unordered_map<Edge, Edge, KeyHasher> combinedEdges;
};
