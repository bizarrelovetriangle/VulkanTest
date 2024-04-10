#include "BoundingBoxObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../CAD/GeometryCreator.h"
#include "../../Renderers/LinedRenderer.h"
#include "../../Renderers/SimpleVertexedRenderer.h"

BoundingBoxObject::BoundingBoxObject(VulkanContext& vulkanContext, const BoundingBox& boundingBox, bool lined)
	: boundingBox(boundingBox)
{
	renderer = lined
		? std::unique_ptr<Renderer>(std::make_unique<LinedRenderer>(vulkanContext))
		: std::unique_ptr<Renderer>(std::make_unique<SimpleVertexedRenderer>(vulkanContext));
	auto center = (boundingBox.aa + boundingBox.bb) / 2;
	mesh = GeometryCreator::CreateBoxByTwoPoints(boundingBox.aa - center, boundingBox.bb - center);

	renderer->transformUniform.model = Matrix4::Translation(center);
	renderer->UpdateTransformUniformBuffer();

	renderer->propertiesUniform.baseColor = {0.1, 0.2, 0.1, 1.};
	renderer->UpdatePropertiesUniformBuffer();

	UpdateVertexBuffer();
}

void BoundingBoxObject::UpdateBoundingBox(const BoundingBox& boundingBox)
{
	auto center = (boundingBox.aa + boundingBox.bb) / 2;
	mesh = GeometryCreator::CreateBoxByTwoPoints(boundingBox.aa - center, boundingBox.bb - center);

	renderer->transformUniform.model = Matrix4::Translation(center);
	renderer->UpdateTransformUniformBuffer();

	renderer->propertiesUniform.baseColor = { 0.1, 0.2, 0.1, 1. };
	renderer->UpdatePropertiesUniformBuffer();

	UpdateVertexBuffer();
}

