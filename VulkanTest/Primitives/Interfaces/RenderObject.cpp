#include "RenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../VulkanContext.h"
#include "../../Utils/GLTFReader.h"

RenderObject::RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
{
	name = deserializedObject.name;
	model = deserializedObject.model;

	uniform.hasTexture = deserializedObject.textureData.has_value();
	uniform.hasColors = deserializedObject.hasColors;
	uniform.baseColor = deserializedObject.baseColor;

	std::span<RenderObjectUniform> uniformSpan(&uniform, &uniform + 1);
	uniformBuffer = std::make_unique<BufferData>(BufferData::Create<RenderObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer));
}

std::vector<vk::DescriptorSetLayoutBinding> RenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll) 
	};
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
