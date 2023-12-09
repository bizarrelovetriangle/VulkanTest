#include "GeometryCreator.h"

std::vector<VertexData> GeometryCreator::createBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb)
{
	Vector3f frontBottomLeft = { aa.x, aa.y, aa.z };
	Vector3f frontBottomRight = { bb.x, aa.y, aa.z };
	Vector3f frontTopRight = { bb.x, bb.y, aa.z };
	Vector3f frontTopLeft = { aa.x, bb.y, aa.z };

	Vector3f backBottomLeft = { aa.x, aa.y, bb.z };
	Vector3f backBottomRight = { bb.x, aa.y, bb.z };
	Vector3f backTopRight = { bb.x, bb.y, bb.z };
	Vector3f backTopLeft = { aa.x, bb.y, bb.z };

	Vector3f top = { 0., 1., 0. };
	Vector3f bottom = { 0.,-1., 0. };
	Vector3f left = { -1., 0., 0. };
	Vector3f right = { 1., 0., 1. };
	Vector3f front = { 0., 0.,-1. };
	Vector3f back = { 0., 0., 1. };

	std::vector<VertexData> vertexData;

	return vertexData;
}

std::vector<LineVertexData> GeometryCreator::createLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb)
{
	Vector3f frontBottomLeft = { aa.x, aa.y, aa.z };
	Vector3f frontBottomRight = { bb.x, aa.y, aa.z };
	Vector3f frontTopRight = { bb.x, bb.y, aa.z };
	Vector3f frontTopLeft = { aa.x, bb.y, aa.z };

	Vector3f backBottomLeft = { aa.x, aa.y, bb.z };
	Vector3f backBottomRight = { bb.x, aa.y, bb.z };
	Vector3f backTopRight = { bb.x, bb.y, bb.z };
	Vector3f backTopLeft = { aa.x, bb.y, bb.z };

	std::vector<LineVertexData> vertexData
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