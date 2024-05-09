#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "../Math/Vector3.h"
#include "BoundingBox.h"
#include <optional>
#include <bitset>
#include "../Utils/FlatHashMap.h"

struct KeyHasher
{
	size_t operator()(const std::pair<uint32_t, uint32_t>& pair) const
	{
		return std::hash<uint32_t>{}(pair.first) ^ std::hash<uint32_t>{}(pair.second);
	}
};

class MeshModel
{
public:
	struct Edge
	{
		uint32_t data = (std::numeric_limits<uint32_t>::max)();

		Edge() {}

		Edge(uint32_t tri, uint32_t side)
		{
			data = (tri << 2) | side;
		}

		uint32_t Triangle() const
		{
			return data >> 2;
		}

		uint32_t Side() const
		{
			return data & 03;
		}
	};

	struct Triangle
	{
		std::array<uint32_t, 3> vertices;
		std::array<Edge, 3> edges;
		uint32_t index;
	};

	MeshModel()
	{}

	MeshModel(const std::vector<uint32_t>& indexes, const std::vector<Vector3f>& points)
	{
		triangles.resize(indexes.size() / 3);
		triangleBitVector.resize(indexes.size() / 3, true);
		markedTris.resize(indexes.size() / 3, false);
		this->points = points;

		for (size_t tri = 0; tri < indexes.size() / 3; ++tri) {
			auto& triangle = triangles.at(tri);
			triangle.index = tri;

			for (int side = 0; side < 3; ++side) {
				size_t org = indexes.at(tri * 3 + side);
				size_t dest = indexes.at(tri * 3 + (side + 1) % 3);
				triangle.vertices[side] = org;
				triangle.edges[side] = Edge(tri, side);
				edges.Insert(std::make_pair(org, dest), triangle.edges[side]);
			}
		}

		localBoundingBox = BoundingBox(*this);
	}

	uint32_t AddTriangle(std::array<uint32_t, 3> indexes)
	{
		uint32_t tri = triangles.size();
		Triangle triangle;
		triangle.index = tri;

		for (int side = 0; side < 3; ++side) {
			size_t org = indexes.at(side);
			size_t dest = indexes.at((side + 1) % 3);
			triangle.vertices[side] = org;
			triangle.edges[side] = Edge(tri, side);

			auto pair = edges.Insert(std::make_pair(org, dest), triangle.edges[side]);
			if (!pair.second) {
				throw std::exception(":(");
			}
		}

		triangles.push_back(triangle);
		triangleBitVector.push_back(true);
		markedTris.push_back(false);
		return tri;
	}

	void DeleteTriangle(uint32_t tri)
	{
		triangleBitVector[tri] = false;
		markedTris[tri] = false;

		auto& triangle = triangles[tri];
		for (auto& edge : triangle.edges) {
			auto org = Origin(edge);
			auto dest = Destination(edge);
			edges.Erase({ org, dest });
		}
	}

	uint32_t Origin(const Edge& edge) const
	{
		auto& triangle = triangles.at(edge.Triangle());
		return triangle.vertices.at(edge.Side());
	}

	uint32_t Destination(const Edge& edge) const
	{
		auto& triangle = triangles.at(edge.Triangle());
		return triangle.vertices.at((edge.Side() + 1) % 3);
	}

	std::optional<Edge> CombinedEdge(const Edge& edge) const
	{
		size_t org = Origin(edge);
		size_t dest = Destination(edge);
		if (auto it = edges.Find(std::make_pair(dest, org)); it != edges.end()) {
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

	void Pack()
	{
		uint32_t seedFace = 0;
		for (; seedFace < triangleBitVector.size() && !triangleBitVector[seedFace]; ++seedFace);
		if (seedFace == triangleBitVector.size()) return;

		std::unordered_set<uint32_t> orgVertices;
		std::vector<uint32_t> orgTriangles;


		for (uint32_t tri = 0; tri < triangleBitVector.size(); ++tri) {
			if (!triangleBitVector[tri]) continue;
			auto triVerteces = triangles[tri].vertices;
			orgTriangles.push_back(tri);
			orgVertices.emplace(triVerteces[0]);
			orgVertices.emplace(triVerteces[1]);
			orgVertices.emplace(triVerteces[2]);
		}

		MeshModel packed;

		std::unordered_map<uint32_t, uint32_t> vertexMap;
		for (auto orgVert : orgVertices) {
			vertexMap.emplace(orgVert, packed.points.size());
			packed.points.push_back(points[orgVert]);
		}

		for (auto orgTri : orgTriangles) {
			auto orgTriVerteces = triangles[orgTri].vertices;
			packed.AddTriangle({ vertexMap[orgTriVerteces[0]], vertexMap[orgTriVerteces[1]], vertexMap[orgTriVerteces[2]] });
		}

		packed.localBoundingBox = BoundingBox(packed);
		*this = packed;
	}

//private:
	std::vector<bool> markedTris;
	std::vector<bool> triangleBitVector;
	std::vector<Triangle> triangles;
	std::vector<Vector3f> points;

	// implement in vector
	FlatHashMap<std::pair<uint32_t, uint32_t>, Edge, KeyHasher> edges;

	BoundingBox localBoundingBox;
};
