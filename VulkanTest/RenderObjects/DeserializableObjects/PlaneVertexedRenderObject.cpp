#include "PlaneVertexedRenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/ImageData.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../RenderVisitor.h"
#include "../../VulkanContext.h"

PlaneVertexedRenderObject::PlaneVertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: DeserializableObject(vulkanContext, deserializedObject)
{
	vertexData = std::vector<VertexData>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	vertexBuffer = BufferData::Create<VertexData>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);

	shared = Shared<PlaneVertexedRenderObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
}
