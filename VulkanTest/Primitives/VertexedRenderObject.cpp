#include "VertexedRenderObject.h"
#include "../Utils/GLTFReader.h"
#include "ColoredRenderObject.h"
#include "TexturedRenderObject.h"
#include "../Vulkan/Memory/DeviceMemory.h"
#include "../RenderVisitor.h"
#include "../Vulkan/Memory/BufferMemory.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/SingletonManager.h"

VertexData::VertexData(const DeserializedObjectVertexData& deserializingObjectVertexData)
{
	position = deserializingObjectVertexData.position;
	normal = deserializingObjectVertexData.normal;
}

vk::VertexInputBindingDescription VertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> VertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, normal));

	return { positionDescription, normalDescription };
}

template <class T>
VertexedRenderObject<T>::VertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: RenderObject(vulkanContext, deserializedObject)
{
	vertexData = std::vector<T>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	vertexBuffer = std::make_unique<BufferMemory<T>>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
}

VertexedRenderObject<VertexData>::VertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: RenderObject(vulkanContext, deserializedObject)
{
	vertexData = std::vector<VertexData>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	vertexBuffer = std::make_unique<BufferMemory<VertexData>>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);

	auto& shared = vulkanContext.singletonManager->Get<Shared<VertexedRenderObject<VertexData>>>();
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared.descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
}

template <class T>
VertexedRenderObject<T>::~VertexedRenderObject() = default;

template <class T>
void VertexedRenderObject<T>::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

template <class T>
void VertexedRenderObject<T>::Dispose()
{
	RenderObject::Dispose();
	vertexBuffer->Dispose();
}

template VertexedRenderObject<ColoredVertexData>;
template VertexedRenderObject<TexturedVertexData>;
template VertexedRenderObject<VertexData>;

