#pragma once
#include "../Interfaces/Object.h"
#include "../../Math/Plane.h"

class PlaneObject : public Object
{
public:
	PlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal);
	Plane plane;
	virtual Matrix4 ComposeMatrix() const override;
};
