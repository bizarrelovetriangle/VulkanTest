#include "MeshModel.h"
#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "../Math/Vector3.h"
#include "BoundingBox.h"
#include "GeometryFunctions.h"

namespace
{
	inline size_t HashPair(const uint32_t& a, const uint32_t& b) noexcept {
		return a << 16 | b;
	}
}

MeshModel::MeshModel()
{}

MeshModel::MeshModel(const std::vector<uint32_t>& indexes, const std::vector<Vector3f>& points)
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
		}
	}

	localBoundingBox = BoundingBox(*this);
}

MeshModel::MeshModel(const MeshModel& meshModel)
{
	markedTris = meshModel.markedTris;
	triangleBitVector = meshModel.triangleBitVector;
	triangles = meshModel.triangles;
	points = meshModel.points;
	//*edges = *meshModel.edges;
	localBoundingBox = meshModel.localBoundingBox;
}

MeshModel& MeshModel::operator=(const MeshModel& meshModel)
{
	markedTris = meshModel.markedTris;
	triangleBitVector = meshModel.triangleBitVector;
	triangles = meshModel.triangles;
	points = meshModel.points;
	//*edges = *meshModel.edges;
	localBoundingBox = meshModel.localBoundingBox;
	return *this;
}

bool MeshModel::Intersect(const std::pair<Vector3f, Vector3f>& line, std::pair<float, Vector3f>* nearestPos) const
{
	if (nearestPos) {
		*nearestPos = { (std::numeric_limits<float>::max)(), {} };
	}

	auto lineDir = (line.second - line.first).Normalized();

	bool success = false;

	float dist = 0.;
	Vector3f intersectPoint;

	for (int i = 0; i < triangles.size(); ++i)
	{
		if (!triangleBitVector[i]) continue;
		auto points = TrianglePoints(i);

		if (GeometryFunctions::SegmentTriangleIntersetion(
			line, lineDir, points[0], points[1], points[2], intersectPoint, &dist))
		{
			if (!nearestPos) {
				return true;
			}

			success = true;
			if (dist < nearestPos->first)
			{
				auto pos = Vector4f(intersectPoint, 1.);
				*nearestPos = { dist, pos };
			}
		}
	}

	return success;
}

uint32_t MeshModel::AddTriangle(std::array<uint32_t, 3> indexes)
{
	uint32_t tri = triangles.size();
	Triangle triangle;
	triangle.index = tri;

	for (int side = 0; side < 3; ++side) {
		size_t org = indexes.at(side);
		size_t dest = indexes.at((side + 1) % 3);
		triangle.vertices[side] = org;
		triangle.edges[side] = Edge(tri, side);

		if (edges) {
			auto pair = edges->emplace(HashPair(org, dest), triangle.edges[side]);
			if (!pair.second) {
				throw std::exception(":(");
			}
		}
	}

	triangles.push_back(triangle);
	triangleBitVector.push_back(true);
	markedTris.push_back(false);
	return tri;
}

void MeshModel::DeleteTriangle(uint32_t tri)
{
	triangleBitVector[tri] = false;
	markedTris[tri] = false;

	auto& triangle = triangles[tri];
	for (auto& edge : triangle.edges) {
		auto org = Origin(edge);
		auto dest = Destination(edge);

		if (edges) {
			edges->erase(HashPair(org, dest));
		}
	}
}

uint32_t MeshModel::Origin(const Edge& edge) const
{
	auto& triangle = triangles.at(edge.Triangle());
	return triangle.vertices.at(edge.Side());
}

uint32_t MeshModel::Destination(const Edge& edge) const
{
	auto& triangle = triangles.at(edge.Triangle());
	return triangle.vertices.at((edge.Side() + 1) % 3);
}

std::optional<MeshModel::Edge> MeshModel::CombinedEdge(const MeshModel::Edge& edge) const
{
	ConstructEdges();

	auto& triangleVerts = triangles.at(edge.Triangle()).vertices;
	auto pair = HashPair(
		triangleVerts.at((edge.Side() + 1) % 3),
		triangleVerts.at(edge.Side()));

	if (auto it = edges->find(pair); it != edges->end()) {
		return it->second;
	}
	return std::nullopt;
}

std::array<Vector3f, 3> MeshModel::TrianglePoints(uint32_t tri) const
{
	const Triangle& triangle = triangles[tri];
	uint32_t a = triangle.vertices[0];
	uint32_t b = triangle.vertices[1];
	uint32_t c = triangle.vertices[2];
	return { points[a], points[b], points[c] };
}

Vector3f MeshModel::TriangleNormal(uint32_t tri) const
{
	const auto& triangle = triangles[tri];
	return (points[triangle.vertices[1]] - points[triangle.vertices[0]])
		.Cross(points[triangle.vertices[2]] - points[triangle.vertices[0]])
		.Normalized();
}

void MeshModel::Pack()
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

void MeshModel::ConstructEdges() const
{
	if (edges) {
		return;
	}

	edges = std::make_unique<FlatHashMap<size_t, Edge>>();

	for (auto& triangle : triangles) {
		for (auto& edge : triangle.edges) {
			size_t org = Origin(edge);
			size_t dest = Destination(edge);
			auto pair = edges->emplace(HashPair(org, dest), edge);
			if (!pair.second) {
				throw std::exception(":(");
			}
		}
	}
}