#include "RenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/Memory/BufferMemory.h"
#undef LoadImage;

RenderObjectVertexData::RenderObjectVertexData(const DeserializedObjectVertexData& deserializingObjectVertexData)
{
	position = deserializingObjectVertexData.position;
	normal = deserializingObjectVertexData.normal;
	textureCoord = deserializingObjectVertexData.textureCoord;
	color = deserializingObjectVertexData.color;
}

vk::VertexInputBindingDescription RenderObjectVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(RenderObjectVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> RenderObjectVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(RenderObjectVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(RenderObjectVertexData, normal));
	vk::VertexInputAttributeDescription textureCoordDescription(
		2, 0, vk::Format::eR32G32Sfloat, offsetof(RenderObjectVertexData, textureCoord));
	vk::VertexInputAttributeDescription colorDescription(
		3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(RenderObjectVertexData, color));

	return { positionDescription, normalDescription, textureCoordDescription, colorDescription };
}


RenderObject::RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
{
	name = deserializedObject.name;
	model = deserializedObject.model;
	vertexData = std::vector<RenderObjectVertexData>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	textureData = deserializedObject.textureData;

	uniform.hasTexture = deserializedObject.textureData.has_value();
	uniform.hasColors = deserializedObject.hasColors;
	uniform.baseColor = deserializedObject.baseColor;

	vertexBuffer = std::make_unique<BufferMemory<RenderObjectVertexData>>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);

	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext);

	if (textureData)
	{
		auto& [resolution, imageData] = *textureData;
		textureBuffer = std::make_unique<ImageMemory>(
			vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
			MemoryType::Universal);
		textureBuffer->FlushData(imageData);
		textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	}
	else
	{
		auto [resolution, imageData] = ImageMemory::LoadImage("E:/Images/testImage.jpeg");
		textureBuffer = std::make_unique<ImageMemory>(
			vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
			MemoryType::Universal);
		textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	std::span<RenderObjectUniform> uniformSpan(&uniform, &uniform + 1);
	uniformBuffer = std::make_unique<BufferMemory<RenderObjectUniform>>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	descriptorSets->UpdateDescriptor(*uniformBuffer, *textureBuffer);
}

RenderObject::~RenderObject() = default;

void RenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

void RenderObject::Dispose()
{
	vertexBuffer->Dispose();
	uniformBuffer->Dispose();
	if (textureBuffer) textureBuffer->Dispose();
	descriptorSets->Dispose();
}

