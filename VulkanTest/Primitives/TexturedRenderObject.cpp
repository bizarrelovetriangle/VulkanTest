#include "TexturedRenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Vulkan/DeviceController.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/Memory/BufferMemory.h"
#include "../Utils/SingletonManager.h"
#undef LoadImage;

TexturedRenderObjectVertexData::TexturedRenderObjectVertexData(
	const DeserializedObjectVertexData& deserializingObjectVertexData)
	: VertexData(deserializingObjectVertexData)
{
	textureCoord = deserializingObjectVertexData.textureCoord;
}

vk::VertexInputBindingDescription TexturedRenderObjectVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(TexturedRenderObjectVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> TexturedRenderObjectVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(TexturedRenderObjectVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(TexturedRenderObjectVertexData, normal));
	vk::VertexInputAttributeDescription textureCoordDescription(
		2, 0, vk::Format::eR32G32Sfloat, offsetof(TexturedRenderObjectVertexData, textureCoord));

	return { positionDescription, normalDescription, textureCoordDescription };
}


TexturedRenderObject::TexturedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: VertexedRenderObject<TexturedRenderObjectVertexData>(vulkanContext, deserializedObject)
{
	textureData = *deserializedObject.textureData;
	auto& [resolution, imageData] = textureData;
	textureBuffer = std::make_unique<ImageMemory>(
		vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
		MemoryType::Universal);
	textureBuffer->FlushData(imageData);
	textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	auto& shared = vulkanContext.singletonManager->Get<Shared<TexturedRenderObject>>();
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared.descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
	descriptorSets->UpdateImageDescriptor(*textureBuffer, 1);
}

std::vector<vk::DescriptorSetLayoutBinding> TexturedRenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
	};
}

TexturedRenderObject::~TexturedRenderObject() = default;

void TexturedRenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

void TexturedRenderObject::Dispose()
{
	VertexedRenderObject<TexturedRenderObjectVertexData>::Dispose();
	textureBuffer->Dispose();
}
