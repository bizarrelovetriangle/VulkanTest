#include "BoundingBox.h"
#include "MeshModel.h"

namespace
{
	constexpr float max = (std::numeric_limits<float>::max)();
}

BoundingBox::BoundingBox()
{

}

BoundingBox::BoundingBox(const MeshModel& mesh)
{
	aa = Vector3f(max, max, max);
	bb = Vector3f(-max, -max, -max);

	for (auto& point : mesh.points)
	{
		if (point.x < aa.x) aa.x = point.x;
		if (point.y < aa.y) aa.y = point.y;
		if (point.z < aa.z) aa.z = point.z;

		if (point.x > bb.x) bb.x = point.x;
		if (point.y > bb.y) bb.y = point.y;
		if (point.z > bb.z) bb.z = point.z;
	}
}

BoundingBox::BoundingBox(const BoundingBox& boundingBox, const Matrix4& model)
{
	auto points = boundingBox.GetPoints();
	for (auto& point : points) {
		point = model * Vector4f(point, 1.);
	}

	aa = Vector3f(max, max, max);
	bb = Vector3f(-max, -max, -max);

	for (auto& point : points)
	{
		if (point.x < aa.x) aa.x = point.x;
		if (point.y < aa.y) aa.y = point.y;
		if (point.z < aa.z) aa.z = point.z;

		if (point.x > bb.x) bb.x = point.x;
		if (point.y > bb.y) bb.y = point.y;
		if (point.z > bb.z) bb.z = point.z;
	}
}

float BoundingBox::GetVolume() const
{
	auto dimentions = bb - aa;
	return dimentions.x * dimentions.y * dimentions.z;
}

std::array<Vector3f, 8> BoundingBox::GetPoints() const
{
	std::array<Vector3f, 8> result;
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 2; ++k)
				result[i * 4 + j * 2 + k] = Vector3f(k ? aa.x : bb.x, j ? aa.y : bb.y, i ? aa.z : bb.z);
	return result;
}

BoundingBox BoundingBox::Union(const BoundingBox& boundingBoxA, const BoundingBox& boundingBoxB)
{
	BoundingBox result;
	result.aa.x = (std::min)(boundingBoxA.aa.x, boundingBoxB.aa.x);
	result.aa.y = (std::min)(boundingBoxA.aa.y, boundingBoxB.aa.y);
	result.aa.z = (std::min)(boundingBoxA.aa.z, boundingBoxB.aa.z);

	result.bb.x = (std::max)(boundingBoxA.bb.x, boundingBoxB.bb.x);
	result.bb.y = (std::max)(boundingBoxA.bb.y, boundingBoxB.bb.y);
	result.bb.z = (std::max)(boundingBoxA.bb.z, boundingBoxB.bb.z);
	return result;
}

void BoundingBox::Reset(const BoundingBox& boundingBoxA, const BoundingBox& boundingBoxB)
{
	aa = Vector3f(max, max, max);
	bb = Vector3f(-max, -max, -max);

	aa.x = (std::min)(boundingBoxA.aa.x, boundingBoxB.aa.x);
	aa.y = (std::min)(boundingBoxA.aa.y, boundingBoxB.aa.y);
	aa.z = (std::min)(boundingBoxA.aa.z, boundingBoxB.aa.z);

	bb.x = (std::max)(boundingBoxA.bb.x, boundingBoxB.bb.x);
	bb.y = (std::max)(boundingBoxA.bb.y, boundingBoxB.bb.y);
	bb.z = (std::max)(boundingBoxA.bb.z, boundingBoxB.bb.z);
}
