#pragma once
#include "MeshModel.h"
#include "unordered_set"
#include "unordered_map"

class Desegmentator
{
public:
	static std::vector<std::shared_ptr<MeshModel>> ConvexSegments(MeshModel mesh)
	{
		std::vector<std::shared_ptr<MeshModel>> result;

		while (true) {
			uint32_t seedFace = 0;
			for (; seedFace < mesh.triangleBitVector.size() && !mesh.triangleBitVector[seedFace]; ++seedFace);
			if (seedFace == mesh.triangleBitVector.size()) break;

			std::unordered_set<uint32_t> orgVertices;
			std::unordered_set<uint32_t> orgTriangles;

			std::unordered_set<uint32_t> candidateTriangles{ seedFace };

			while (!candidateTriangles.empty() && orgTriangles.size() < 500000) {
				uint32_t face = *candidateTriangles.begin();
				candidateTriangles.erase(candidateTriangles.begin());

				auto triVerteces = mesh.triangles[face].vertices;
				orgTriangles.emplace(face);
				orgVertices.emplace(triVerteces[0]);
				orgVertices.emplace(triVerteces[1]);
				orgVertices.emplace(triVerteces[2]);

				for (auto& edge : mesh.triangles[face].edges) {
					if (auto combEdge = mesh.CombinedEdge(edge); combEdge) {
						if (!orgTriangles.contains(combEdge->Triangle())) {
							candidateTriangles.emplace(combEdge->Triangle());
						}
					}
				}

				mesh.DeleteTriangle(face);
			}

			auto segment = std::make_shared<MeshModel>();

			std::unordered_map<uint32_t, uint32_t> vertexMap;
			for (auto orgVert : orgVertices) {
				vertexMap.emplace(orgVert, segment->points.size());
				segment->points.push_back(mesh.points[orgVert]);
			}

			for (auto orgTri : orgTriangles) {
				auto orgTriVerteces = mesh.triangles[orgTri].vertices;
				segment->AddTriangle({ vertexMap[orgTriVerteces[0]], vertexMap[orgTriVerteces[1]], vertexMap[orgTriVerteces[2]] });
			}

			segment->localBoundingBox = BoundingBox(*segment);
			result.push_back(std::move(segment));
		}

		return result;
	}
};
