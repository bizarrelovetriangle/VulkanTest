#pragma once
#include "../Interfaces/MeshObject.h"
#include "../../Math/Vector3.h"
#include "../../Math/Plane.h"


class BoundingBoxObject : public MeshObject
{
public:
	// aa - front bottom left
	// bb - back top right
	BoundingBoxObject(VulkanContext& vulkanContext, const Vector3f& aa, const Vector3f& bb);

private:
	std::array<Vector3f, 8> boundingPoints;
	std::array<Vector3f, 8> expandedBoundingPoints;
};
