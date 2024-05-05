#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "../Math/Vector3.h"
#include "BoundingBox.h"
#include <optional>
#include <bitset>

class MeshModel
{
public:
	struct Edge
	{
		size_t triangleIndex = 0;
		size_t side = 0;

		//friend bool operator==(const Edge& a, const Edge& b) {
		//	return a.triangleIndex == b.triangleIndex && a.side == b.side;
		//}
	};

	struct Triangle
	{
		std::array<uint32_t, 3> vertices;
		std::array<Edge, 3> edges;
		uint32_t index;
	};

	struct KeyHasher
	{
		size_t operator()(const std::pair<uint32_t, uint32_t>& pair) const
		{
			return std::hash<uint32_t>{}(pair.first) ^ std::hash<uint32_t>{}(pair.second);
		}

		//size_t operator()(const Edge& edge) const
		//{
		//	return std::hash<uint32_t>{}(edge.triangleIndex) ^ std::hash<uint32_t>{}(edge.side);
		//}
	};

	MeshModel()
	{}

	MeshModel(const std::vector<uint32_t>& indexes, const std::vector<Vector3f>& points)
	{
		triangles.resize(indexes.size() / 3);
		triangleBitVector.resize(indexes.size() / 3, true);
		markedTris.resize(indexes.size() / 3, false);
		this->points = points;

		for (size_t i = 0; i < indexes.size() / 3; ++i) {
			auto& triangle = triangles.at(i);
			triangle.index = i;

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

	uint32_t AddTriangle(std::array<uint32_t, 3> indexes)
	{
		uint32_t triIndex = triangles.size();
		Triangle triangle;
		triangle.index = triIndex;

		for (int j = 0; j < 3; ++j) {
			size_t org = indexes.at(j);
			size_t dest = indexes.at((j + 1) % 3);
			triangle.vertices[j] = org;
			triangle.edges[j].triangleIndex = triIndex;
			triangle.edges[j].side = j;

			auto pair = edges.emplace(std::make_pair(org, dest), triangle.edges[j]);
			if (!pair.second) {
				throw std::exception(":(");
			}
		}

		triangles.push_back(triangle);
		triangleBitVector.push_back(true);
		markedTris.push_back(false);
		return triIndex;
	}

	void DeleteTriangle(uint32_t tri)
	{
		triangleBitVector[tri] = false;
		markedTris[tri] = false;

		auto& triangle = triangles[tri];
		for (auto& edge : triangle.edges) {
			auto org = Origin(edge);
			auto dest = Destination(edge);
			edges.erase({ org, dest });
		}
	}

	uint32_t Origin(const Edge& edge) const
	{
		auto& triangle = triangles.at(edge.triangleIndex);
		return triangle.vertices.at(edge.side);
	}

	uint32_t Destination(const Edge& edge) const
	{
		auto& triangle = triangles.at(edge.triangleIndex);
		return triangle.vertices.at((edge.side + 1) % 3);
	}

	std::optional<Edge> CombinedEdge(const Edge& edge) const
	{
		size_t org = Origin(edge);
		size_t dest = Destination(edge);
		if (auto it = edges.find(std::make_pair(dest, org)); it != edges.end()) {
			return it->second;
		}
		return std::nullopt;
	}

	std::array<Vector3f, 3> TrianglePoints(uint32_t tri) const
	{
		const Triangle& triangle = triangles[tri];
		uint32_t a = triangle.vertices[0];
		uint32_t b = triangle.vertices[1];
		uint32_t c = triangle.vertices[2];
		return { points[a], points[b], points[c] };
	}

	Vector3f TriangleNormal(uint32_t tri) const
	{
		const auto& triangle = triangles[tri];
		return (points[triangle.vertices[1]] - points[triangle.vertices[0]])
			.Cross(points[triangle.vertices[2]] - points[triangle.vertices[0]])
			.Normalized();
	}

//private:
	std::vector<bool> markedTris;
	std::vector<bool> triangleBitVector;
	std::vector<Triangle> triangles;
	std::vector<Vector3f> points;

	std::unordered_map<std::pair<uint32_t, uint32_t>, Edge, KeyHasher> edges;

	BoundingBox localBoundingBox;
};
