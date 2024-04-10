#include "PlaneObject.h"
#include "../../Renderers/PlaneRenderer.h"

PlaneObject::PlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal)
{
	plane = Plane(position, normal);

	float scale = 3.;
	std::vector<Vector3f> positions { {-1., 0., -1.}, {1., 0., -1.}, {1., 0., 1.}, {-1., 0., 1.} };
	for (auto& pos : positions) pos = pos * scale;
	std::vector<uint32_t> indexes{ 0, 1, 2, 2, 3, 0 };
	mesh = std::make_unique<MeshModel>(indexes, positions);

	auto planeRenderer = std::make_unique<PlaneRenderer>(vulkanContext);

	planeRenderer->transformUniform.model = plane.getMatrix();
	planeRenderer->UpdateTransformUniformBuffer();

	planeRenderer->evenPlaneObjectUniform.color = Vector4f(0.5, 0., 0.5, 1.);
	planeRenderer->UpdatePlaneUniformBuffer();

	renderer = std::move(planeRenderer);

	UpdateVertexBuffer();
}

Matrix4 PlaneObject::ComposeMatrix() const
{
	return plane.getMatrix();
}