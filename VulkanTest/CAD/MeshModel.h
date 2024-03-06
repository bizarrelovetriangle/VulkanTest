#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "../Math/Vector3.h"
#include "BoundingBox.h"
#include <optional>

struct Edge
{
	size_t triangleIndex = 0;
	size_t side = 0;

	friend bool operator==(const Edge& a, const Edge& b) {
		return a.triangleIndex == b.triangleIndex && a.side == b.side;
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
	MeshModel()
	{}

	MeshModel(const std::vector<uint32_t>& indexes, const std::vector<Vector3f>& points)
	{
		triangles.resize(indexes.size() / 3);
		this->points = points;

		for (size_t i = 0; i < indexes.size() / 3; ++i) {
			auto& triangle = triangles.at(i);

			for (int j = 0; j < 3; ++j) {
				size_t org = indexes.at(i * 3 + j);
				size_t dest = indexes.at(i * 3 + (j + 1) % 3);
				triangle.vertices[j] = org;
				triangle.edges[j].triangleIndex = i;
				triangle.edges[j].side = j;
				edges.emplace(std::make_pair(org, dest), triangle.edges[j]);
			}
		}

		localBoundingBox = BoundingBox(*this);
	}

	void AddTriangle(std::array<uint32_t, 3> indexes)
	{
		uint32_t triIndex = triangles.size();
		Triangle triangle;

		for (int j = 0; j < 3; ++j) {
			size_t org = indexes.at(j);
			size_t dest = indexes.at((j + 1) % 3);
			triangle.vertices[j] = org;
			triangle.edges[j].triangleIndex = triIndex;
			triangle.edges[j].side = j;
			edges.emplace(std::make_pair(org, dest), triangle.edges[j]);
		}

		triangles.push_back(triangle);
	}

	void DeleteTriangle(uint32_t triangle)
	{
		// todo: using of bitvector?
		triangles.erase(triangles.begin() + triangle);
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

	std::optional<Edge> CombinedEdge(const Edge& edge)
	{
		size_t org = Origin(edge);
		size_t dest = Destination(edge);
		if (auto it = edges.find(std::make_pair(org, dest)); it != edges.end()) {
			return it->second;
		}
		return std::nullopt;
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

	std::unordered_map<std::pair<uint32_t, uint32_t>, Edge, KeyHasher> edges;

	BoundingBox localBoundingBox;
};
