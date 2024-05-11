#pragma once
#include <limits>
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"

class BoundingBoxObject;
class MeshModel;
class MeshObject;

class BoundingBox
{
public:
	BoundingBox();
	BoundingBox(const MeshModel& mesh);
	BoundingBox(const BoundingBox& boundingBox, const Matrix4& model, float offset = 0.);

	float GetVolume() const;
	std::array<Vector3f, 8> GetPoints() const;

	bool Intersect(const BoundingBox& box) const;
	bool Intersect(const std::pair<Vector3f, Vector3f>& line) const;
	bool Exceed(const BoundingBox& exceededBox) const;
	static BoundingBox Union(const BoundingBox& boundingBoxA, const BoundingBox& boundingBoxB);
	void Reset(const BoundingBox& boundingBoxA, const BoundingBox& boundingBoxB);

	Vector3f aa;
	Vector3f bb;

	int64_t parent = -1;
	std::pair<int64_t, int64_t> children = std::make_pair(-1, -1);
	std::shared_ptr<MeshObject> sceneObject;

	std::shared_ptr<BoundingBoxObject> renderBoundingBoxObject;
};
