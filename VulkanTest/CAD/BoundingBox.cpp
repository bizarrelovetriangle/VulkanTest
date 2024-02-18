#include "BoundingBox.h"
#include "MeshModel.h"

namespace
{
	constexpr float min = (std::numeric_limits<float>::min)();
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
		auto vec4 = model * Vector4f(point, 1.);
		point = Vector3f(vec4.x, vec4.y, vec4.z);
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

std::array<Vector3f, 8> BoundingBox::GetPoints() const
{
	std::array<Vector3f, 8> result;
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 2; ++k)
				result[i * 4 + j * 2 + k] = Vector3f(k ? aa.x : bb.x, j ? aa.y : bb.y, i ? aa.z : bb.z);
	return result;
}

