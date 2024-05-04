#include "GeometryCreator.h"
#include "../CAD/MeshModel.h"
#include <map>

std::unique_ptr<MeshModel> GeometryCreator::CreateIcosphere(float radius, int subdivisions)
{
	const float X = radius * .525731112119133606f;
	const float Z = radius * .850650808352039932f;
	const float N = radius * 0.f;

	std::vector<Vector3f> positions
	{
	  {-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
	  {N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
	  {Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
	};

	std::vector<uint32_t> indexes
	{
		0, 4, 1,  0, 9, 4,  9, 5, 4,  4, 5, 8,  4, 8, 1,
		8,10, 1,  8, 3,10,  5, 3, 8,  5, 2, 3,  2, 7, 3,
		7,10, 3,  7, 6,10,  7,11, 6, 11, 0, 6,  0, 1, 6,
		6, 1,10,  9, 0,11,  9,11, 2,  9, 2, 5,  7, 2,11
	};

	std::unique_ptr<MeshModel> icosphere = std::make_unique<MeshModel>(indexes, positions);
	std::unique_ptr<MeshModel> icosphereTemp;

	for (int subd = 0; subd < subdivisions; ++subd)
	{
		icosphereTemp = std::make_unique<MeshModel>(std::vector<uint32_t>(), icosphere->points);

		std::map<std::pair<uint32_t, uint32_t>, uint32_t> edgeSplitMap;

		for (const auto& tri : icosphere->triangles) {
			std::vector<uint32_t> newVert(3);

			for (int edge = 0; edge < 3; ++edge) {
				uint32_t vertA = tri.vertices[edge];
				uint32_t vertB = tri.vertices[(edge + 1) % 3];

				auto pair = std::make_pair(vertA, vertB);
				if (pair.first > pair.second) std::swap(pair.first, pair.second);

				auto it = edgeSplitMap.emplace(pair, icosphereTemp->points.size());

				if (it.second) {
					auto split = (icosphere->points[vertA] + icosphere->points[vertB]).Normalized() * radius;
					icosphereTemp->points.push_back(split);
				}

				newVert[edge] = it.first->second;
			}

			icosphereTemp->AddTriangle({ tri.vertices[0], newVert[0], newVert[2] });
			icosphereTemp->AddTriangle({ tri.vertices[1], newVert[1], newVert[0] });
			icosphereTemp->AddTriangle({ tri.vertices[2], newVert[2], newVert[1] });
			icosphereTemp->AddTriangle({ newVert[0], newVert[1], newVert[2] });
		}

		std::swap(icosphere, icosphereTemp);
	}

	return icosphere;
}

std::unique_ptr<MeshModel> GeometryCreator::CreateBox(const Vector3f& size)
{
	return CreateBoxByTwoPoints(Vector3f::Zero() - size / 2, Vector3f::Zero() + size / 2);
}

std::unique_ptr<MeshModel> GeometryCreator::CreateBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb)
{
	std::vector<Vector3f> positions{
		{ aa.x, aa.y, aa.z },   //0
		{ bb.x, aa.y, aa.z },   //1
		{ bb.x, bb.y, aa.z },   //2
		{ aa.x, bb.y, aa.z },   //3
		{ aa.x, aa.y, bb.z },   //4
		{ bb.x, aa.y, bb.z },   //5
		{ bb.x, bb.y, bb.z },   //6
		{ aa.x, bb.y, bb.z } }; //7

	std::vector<uint32_t> indexes{
		//front
		0, 1, 2,
		2, 3, 0,
		//back
		6, 5, 4,
		4, 7, 6,
		//top
		3, 2, 6,
		6, 7, 3,
		//bottom
		5, 1, 0,
		0, 4, 5,
		//left
		3, 7, 4,
		4, 0, 3,
		//right
		2, 1, 5,
		5, 6, 2
	};

	auto mesh = std::make_unique<MeshModel>(indexes, positions);
	return mesh;
}

std::unique_ptr<MeshModel> GeometryCreator::CreateLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb)
{
	std::vector<Vector3f> positions{
		{ aa.x, aa.y, aa.z },   //0
		{ bb.x, aa.y, aa.z },   //1
		{ bb.x, bb.y, aa.z },   //2
		{ aa.x, bb.y, aa.z },   //3
		{ aa.x, aa.y, bb.z },   //4
		{ bb.x, aa.y, bb.z },   //5
		{ bb.x, bb.y, bb.z },   //6
		{ aa.x, bb.y, bb.z } }; //7

	std::vector<uint32_t> indexes
	{
		//front
		0, 1, 1,
		1, 2, 2,
		2, 3, 3,
		3, 0, 0,

		//back
		4, 5, 5,
		5, 6, 6,
		6, 7, 7,
		7, 4, 4,

		//edjes
		0, 4, 4,
		1, 5, 5,
		2, 6, 6,
		3, 7, 7,
	};

	auto mesh = std::make_unique<MeshModel>(indexes, positions);
	return mesh;
}