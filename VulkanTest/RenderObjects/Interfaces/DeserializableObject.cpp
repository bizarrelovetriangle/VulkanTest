#include "DeserializableObject.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"

DeserializableObject::DeserializableObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: RenderObject(vulkanContext)
{
	name = deserializedObject.name;
	model = deserializedObject.model;

	uniform.hasTexture = deserializedObject.textureData.has_value();
	uniform.hasColors = deserializedObject.hasColors;
	uniform.baseColor = deserializedObject.baseColor;

	std::span<DeserializableObjectUniform> uniformSpan(&uniform, &uniform + 1);
	uniformBuffer = std::make_unique<BufferData>(BufferData::Create<DeserializableObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer));
}
