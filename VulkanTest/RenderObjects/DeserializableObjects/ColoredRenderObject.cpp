#include "ColoredRenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/ImageData.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../RenderVisitor.h"
#include "../../VulkanContext.h"

ColoredVertexData::ColoredVertexData(const DeserializedObjectVertexData& deserializingObjectVertexData)
	: VertexData(deserializingObjectVertexData)
{
	color = deserializingObjectVertexData.color;
}

vk::VertexInputBindingDescription ColoredVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(ColoredVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> ColoredVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(ColoredVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(ColoredVertexData, normal));
	vk::VertexInputAttributeDescription colorDescription(
		2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(ColoredVertexData, color));

	return { positionDescription, normalDescription, colorDescription };
}


ColoredRenderObject::ColoredRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: VertexedRenderObject(vulkanContext, deserializedObject)
{
	vertexData = std::vector<ColoredVertexData>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	vertexBuffer = std::make_unique<BufferData>(BufferData::Create<ColoredVertexData>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer));

	shared = Shared<ColoredRenderObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
}
