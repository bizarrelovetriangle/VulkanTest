#include "BoundingBoxObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../Utils/GeometryCreator.h"
#include "../../RenderObjects/LinedRenderObject.h"

BoundingBoxObject::BoundingBoxObject(VulkanContext& vulkanContext, const BoundingBox& boundingBox)
	: boundingBox(boundingBox)
{
	renderer = std::make_unique<LinedRenderObject>(vulkanContext);
	auto center = (boundingBox.aa + boundingBox.bb) / 2;
	mesh = GeometryCreator::createBoxByTwoPoints(boundingBox.aa - center, boundingBox.bb - center);

	renderer->transformUniform.model = Matrix4::Translation(center);
	renderer->UpdateTransformUniformBuffer();

	renderer->propertiesUniform.baseColor = {0.1, 0.2, 0.1, 1.};
	renderer->UpdatePropertiesUniformBuffer();

	mesh = GeometryCreator::createBoxByTwoPoints(boundingBox.aa - center, boundingBox.bb - center);
	UpdateVertexBuffer();
}

