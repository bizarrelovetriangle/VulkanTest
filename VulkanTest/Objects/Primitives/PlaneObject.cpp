#include "PlaneObject.h"
#include "../../RenderObjects/PlaneRenderObject.h"

PlaneObject::PlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal)
{
	plane = Plane(position, normal);

	auto planeRenderer = std::make_unique<PlaneRenderObject>(vulkanContext);

	planeRenderer->transformUniform.model = plane.getMatrix();
	planeRenderer->UpdateTransformUniformBuffer();

	planeRenderer->evenPlaneObjectUniform.color = Vector4f(0.5, 0., 0.5, 1.);
	planeRenderer->UpdatePlaneUniformBuffer();

	renderer = std::move(planeRenderer);
}

Matrix4 PlaneObject::ComposeMatrix() const
{
	return plane.getMatrix();
}