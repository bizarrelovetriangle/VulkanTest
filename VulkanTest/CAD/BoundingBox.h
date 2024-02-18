#pragma once
#include <limits>
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"

class BoundingBoxObject;
class MeshModel;
class Object;

class BoundingBox
{
public:
	BoundingBox();
	BoundingBox(const MeshModel& mesh);
	BoundingBox(const BoundingBox& boundingBox, const Matrix4& model);

	std::array<Vector3f, 8> GetPoints() const;

	Vector3f aa;
	Vector3f bb;

	std::shared_ptr<BoundingBox> parent;
	std::array<std::shared_ptr<BoundingBox>, 2> children;
	std::shared_ptr<Object> sceneObject;

	std::shared_ptr<BoundingBoxObject> renderBoundingBoxObject;
};
