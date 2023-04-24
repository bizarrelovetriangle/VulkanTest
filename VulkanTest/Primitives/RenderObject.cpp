#include "RenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/Memory/BufferMemory.h"

RenderObject::RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
{
	name = deserializedObject.name;
	model = deserializedObject.model;

	uniform.hasTexture = deserializedObject.textureData.has_value();
	uniform.hasColors = deserializedObject.hasColors;
	uniform.baseColor = deserializedObject.baseColor;

	std::span<RenderObjectUniform> uniformSpan(&uniform, &uniform + 1);
	uniformBuffer = std::make_unique<BufferMemory<RenderObjectUniform>>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	std::vector<vk::DescriptorSetLayoutBinding> bindings
	{
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, bindings);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
}

RenderObject::~RenderObject() = default;

void RenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

void RenderObject::Dispose()
{
	uniformBuffer->Dispose();
	descriptorSets->Dispose();
}

