#include "BoundingBoxObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../Utils/GeometryCreator.h"


LineVertexData::LineVertexData(const Vector3f& position)
	: position(position)
{
}

vk::VertexInputBindingDescription LineVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(LineVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> LineVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(LineVertexData, position));
	return { positionDescription };
}

BoundingBoxObject::BoundingBoxObject(VulkanContext& vulkanContext, const Vector3f& aa, const Vector3f& bb)
	: VertexedRenderObject(vulkanContext)
{
	auto center = (aa + bb) / 2;
	transformUniform.model = Matrix4::Translation(center);
	UpdateTransformUniformBuffer();

	propertiesUniform.baseColor = {0.1, 0.2, 0.1, 1.};
	UpdatePropertiesUniformBuffer();

	vertexData = GeometryCreator::createLinedBoxByTwoPoints(aa - center, bb - center);
	vertexBuffer = std::make_unique<BufferData>(BufferData::Create<LineVertexData>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer));

	shared = Shared<BoundingBoxObject>::getInstance(vulkanContext, true);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
}

BoundingBoxObject::~BoundingBoxObject() = default;
