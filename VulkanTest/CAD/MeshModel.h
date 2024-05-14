#pragma once
#include <vector>
#include <array>
#include "../Math/Vector3.h"
#include "BoundingBox.h"
#include <optional>
#include <unordered_map>
#include "../Utils/FlatHashMap.h"

struct KeyHasher
{
	size_t operator()(const std::pair<uint32_t, uint32_t>& pair) const
	{
		return pair.first ^ pair.second;
	}

	size_t operator()(const size_t& v) const
	{
		return v;
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

	MeshModel();

	MeshModel(const std::vector<uint32_t>& indexes, const std::vector<Vector3f>& points);

	MeshModel(const MeshModel& meshModel);

	MeshModel& operator=(const MeshModel& meshModel);

	bool Intersect(const std::pair<Vector3f, Vector3f>& line, std::pair<float, Vector3f>* nearestPos = nullptr) const;

	uint32_t AddTriangle(std::array<uint32_t, 3> indexes);

	void DeleteTriangle(uint32_t tri);

	uint32_t Origin(const Edge& edge) const;

	uint32_t Destination(const Edge& edge) const;

	std::optional<Edge> CombinedEdge(const Edge& edge) const;

	std::array<Vector3f, 3> TrianglePoints(uint32_t tri) const;

	Vector3f TriangleNormal(uint32_t tri) const;

	void Pack();

	void ConstructEdges() const;

//private:
	std::vector<uint32_t> markedTris;
	std::vector<uint32_t> triangleBitVector;
	std::vector<Triangle> triangles;
	std::vector<Vector3f> points;


	mutable std::unique_ptr<FlatHashMap<size_t, Edge>> edges;

	BoundingBox localBoundingBox;
};
