#pragma once
#include "../Interfaces/MeshObject.h"
#include "../../Math/Plane.h"

class PlaneObject : public MeshObject
{
public:
	PlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal);
	virtual Matrix4 ComposeMatrix() const override;
	Plane plane;
};
