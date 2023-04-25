#include "ColoredRenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/Memory/BufferMemory.h"
#include "../RenderVisitor.h"
#include "../Utils/SingletonManager.h"

ColoredRenderObjectVertexData::ColoredRenderObjectVertexData(
	const DeserializedObjectVertexData& deserializingObjectVertexData)
	: VertexData(deserializingObjectVertexData)
{
	color = deserializingObjectVertexData.color;
}

vk::VertexInputBindingDescription ColoredRenderObjectVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(ColoredRenderObjectVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> ColoredRenderObjectVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(ColoredRenderObjectVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(ColoredRenderObjectVertexData, normal));
	vk::VertexInputAttributeDescription colorDescription(
		2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(ColoredRenderObjectVertexData, color));

	return { positionDescription, normalDescription, colorDescription };
}


ColoredRenderObject::ColoredRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: VertexedRenderObject<ColoredRenderObjectVertexData>(vulkanContext, deserializedObject)
{
	auto& shared = vulkanContext.singletonManager->Get<Shared<ColoredRenderObject>>();
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared.descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
}

ColoredRenderObject::~ColoredRenderObject() = default;

void ColoredRenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}
