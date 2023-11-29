#pragma once
#include "../Interfaces/RenderObject.h"
#include "../../Math/Vector3.h"
#include "../../Math/Plane.h"

class RenderVisitor;

class EvenPlaneObjectUniform
{
public:
	alignas(16) Vector4f color;
	alignas(4) bool grided = true;
};

class EvenPlaneObject : public RenderObject
{
public:
	EvenPlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal);
	virtual void Accept(RenderVisitor& renderVisitor) const override;

public:
	Plane plane;

	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Primitives/EvenPlane.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Primitives/EvenPlane.frag";

	EvenPlaneObjectUniform uniform;
};
