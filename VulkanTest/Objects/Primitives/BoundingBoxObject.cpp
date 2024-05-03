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
	mesh = GeometryCreator::CreateBoxByTwoPoints(boundingBox.aa, boundingBox.bb);

	renderer->propertiesUniform.baseColor = {0.1, 0.2, 0.1, 1.};
	renderer->UpdatePropertiesUniformBuffer();

	UpdateVertexBuffer();
}

void BoundingBoxObject::UpdateBoundingBox(const BoundingBox& boundingBox)
{
	mesh = GeometryCreator::CreateBoxByTwoPoints(boundingBox.aa, boundingBox.bb);

	renderer->propertiesUniform.baseColor = { 0.1, 0.2, 0.1, 1. };
	renderer->UpdatePropertiesUniformBuffer();

	UpdateVertexBuffer();
}

