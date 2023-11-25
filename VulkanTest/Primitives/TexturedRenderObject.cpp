#include "TexturedRenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Vulkan/DeviceController.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../VulkanContext.h"
#undef LoadImage;

TexturedVertexData::TexturedVertexData(
	const DeserializedObjectVertexData& deserializingObjectVertexData)
	: VertexData(deserializingObjectVertexData)
{
	textureCoord = deserializingObjectVertexData.textureCoord;
}

vk::VertexInputBindingDescription TexturedVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(TexturedVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> TexturedVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(TexturedVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(TexturedVertexData, normal));
	vk::VertexInputAttributeDescription textureCoordDescription(
		2, 0, vk::Format::eR32G32Sfloat, offsetof(TexturedVertexData, textureCoord));

	return { positionDescription, normalDescription, textureCoordDescription };
}


TexturedRenderObject::TexturedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: VertexedRenderObject(vulkanContext, deserializedObject)
{
	vertexData = std::vector<TexturedVertexData>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	vertexBuffer = std::make_unique<BufferData>(BufferData::Create<TexturedVertexData>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer));

	textureData = *deserializedObject.textureData;
	auto& [resolution, imageData] = textureData;
	textureBuffer = std::make_unique<ImageData>(
		vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
		MemoryType::Universal);
	textureBuffer->FlushData(imageData);
	textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	shared = Shared<TexturedRenderObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
	descriptorSets->UpdateImageDescriptor(*textureBuffer, 1);
}

TexturedRenderObject::~TexturedRenderObject() = default;

std::vector<vk::DescriptorSetLayoutBinding> TexturedRenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
	};
}

void TexturedRenderObject::Dispose()
{
	VertexedRenderObject::Dispose();
	textureBuffer->Dispose();
}
