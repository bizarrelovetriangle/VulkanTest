#include "PlaneObject.h"
#include "../../Renderers/PlaneRenderer.h"

PlaneObject::PlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal)
{
	this->position = position;
	plane = Plane(position, normal);

	std::vector<Vector3f> positions { {-1., 0., -1.}, {1., 0., -1.}, {1., 0., 1.}, {-1., 0., 1.} };
	std::vector<uint32_t> indexes{ 0, 1, 2, 2, 3, 0 };
	mesh = std::make_unique<MeshModel>(indexes, positions);

	auto planeRenderer = std::make_unique<PlaneRenderer>(vulkanContext);

	planeRenderer->transformUniform.model = plane.getMatrix();
	planeRenderer->UpdateTransformUniformBuffer();

	planeRenderer->evenPlaneObjectUniform.color = Vector4f(0.1, 0.1, 0.2, 1.);
	planeRenderer->UpdatePlaneUniformBuffer();

	renderer = std::move(planeRenderer);

	UpdateVertexBuffer();
}

Matrix4 PlaneObject::ComposeMatrix() const
{
	auto planeOffset = position - plane.normal * plane.normal.Dot(position);
	return Matrix4::Translation(planeOffset) * plane.getMatrix() * Matrix4::Scale(scale);
}