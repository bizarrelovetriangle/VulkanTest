#include "GeometryCreator.h"
#include "../CAD/MeshModel.h"

std::unique_ptr<MeshModel> GeometryCreator::createBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb)
{
	std::vector<Vector3f> positions{
		{ aa.x, aa.y, aa.z },   //0
		{ bb.x, aa.y, aa.z },
		{ bb.x, bb.y, aa.z },
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

	auto&& mesh = std::make_unique<MeshModel>(indexes, positions);
	return mesh;
}

std::vector<Vector3f> GeometryCreator::createLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb)
{
	Vector3f frontBottomLeft = { aa.x, aa.y, aa.z };
	Vector3f frontBottomRight = { bb.x, aa.y, aa.z };
	Vector3f frontTopRight = { bb.x, bb.y, aa.z };
	Vector3f frontTopLeft = { aa.x, bb.y, aa.z };

	Vector3f backBottomLeft = { aa.x, aa.y, bb.z };
	Vector3f backBottomRight = { bb.x, aa.y, bb.z };
	Vector3f backTopRight = { bb.x, bb.y, bb.z };
	Vector3f backTopLeft = { aa.x, bb.y, bb.z };

	std::vector<Vector3f> vertexData
	{
		//front
		frontBottomLeft, frontBottomRight,
		frontBottomRight, frontTopRight,
		frontTopRight, frontTopLeft,
		frontTopLeft, frontBottomLeft,

		//back
		backBottomLeft, backBottomRight,
		backBottomRight, backTopRight,
		backTopRight, backTopLeft,
		backTopLeft, backBottomLeft,

		//edjes
		frontBottomLeft, backBottomLeft,
		frontBottomRight, backBottomRight,
		frontTopRight, backTopRight,
		frontTopLeft, backTopLeft,
	};

	return vertexData;
}