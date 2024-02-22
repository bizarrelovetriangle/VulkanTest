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
	BoundingBox(const BoundingBox& boundingBox, const Matrix4& model);

	std::array<Vector3f, 8> GetPoints() const;

	static BoundingBox Union(const BoundingBox& boundingBoxA, const BoundingBox& boundingBoxB);

	Vector3f aa;
	Vector3f bb;

	size_t parent;
	std::array<size_t, 2> children;
	std::shared_ptr<MeshObject> sceneObject;

	std::shared_ptr<BoundingBoxObject> renderBoundingBoxObject;
};
