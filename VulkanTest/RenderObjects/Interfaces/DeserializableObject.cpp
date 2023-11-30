#include "DeserializableObject.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"

DeserializableObject::DeserializableObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: RenderObject(vulkanContext)
{
	gltfName = deserializedObject.name;
	transformUniform.model = deserializedObject.model;
	UpdateTransformUniformBuffer();

	deserializableUniform.hasTexture = deserializedObject.textureData.has_value();
	deserializableUniform.hasColors = deserializedObject.hasColors;
	deserializableUniform.baseColor = deserializedObject.baseColor;

	std::span<DeserializableObjectUniform> uniformSpan(&deserializableUniform, &deserializableUniform + 1);
	deserializableUniformBuffer = std::make_unique<BufferData>(BufferData::Create<DeserializableObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer));
}

DeserializableObject::~DeserializableObject() = default;

void DeserializableObject::Dispose()
{
	RenderObject::Dispose();
	deserializableUniformBuffer->Dispose();
}

std::vector<vk::DescriptorSetLayoutBinding> DeserializableObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}