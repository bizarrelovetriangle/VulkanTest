#include "BoundingBoxObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../Utils/GeometryCreator.h"
#include "../../RenderObjects/LinedRenderObject.h"

BoundingBoxObject::BoundingBoxObject(VulkanContext& vulkanContext, const Vector3f& aa, const Vector3f& bb)
{
	renderer = std::make_unique<LinedRenderObject>(vulkanContext);

	auto center = (aa + bb) / 2;
	renderer->transformUniform.model = Matrix4::Translation(center);
	renderer->UpdateTransformUniformBuffer();

	renderer->propertiesUniform.baseColor = {0.1, 0.2, 0.1, 1.};
	renderer->UpdatePropertiesUniformBuffer();

	mesh = GeometryCreator::createBoxByTwoPoints(aa - center, bb - center);
	UpdateVertexBuffer();
}

