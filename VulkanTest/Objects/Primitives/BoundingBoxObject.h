#pragma once
#include "../Interfaces/MeshObject.h"
#include "../../Math/Vector3.h"
#include "../../Math/Plane.h"
#include "../../CAD/BoundingBox.h"


class BoundingBoxObject : public MeshObject
{
public:
	// aa - front bottom left
	// bb - back top right
	BoundingBoxObject(VulkanContext& vulkanContext, const BoundingBox& boundingBox, bool lined = true);
	void UpdateBoundingBox(const BoundingBox& boundingBox);
private:
	BoundingBox boundingBox;
};
