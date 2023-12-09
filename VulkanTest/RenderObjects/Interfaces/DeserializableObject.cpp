#include "DeserializableObject.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"

DeserializableObject::DeserializableObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: VertexedRenderObject(vulkanContext)
{
	gltfName = deserializedObject.name;
	transformUniform.model = deserializedObject.model;
	UpdateTransformUniformBuffer();

	propertiesUniform.hasTexture = deserializedObject.textureData.has_value();
	propertiesUniform.hasColors = deserializedObject.hasColors;
	propertiesUniform.baseColor = deserializedObject.baseColor;
	UpdatePropertiesUniformBuffer();
}

DeserializableObject::~DeserializableObject() = default;

void DeserializableObject::Dispose()
{
	VertexedRenderObject::Dispose();
}

std::vector<vk::DescriptorSetLayoutBinding> DeserializableObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}