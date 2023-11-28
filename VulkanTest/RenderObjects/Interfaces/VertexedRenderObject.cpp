#include "VertexedRenderObject.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/DeviceMemory/DeviceMemory.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"

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


VertexedRenderObject::VertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: DeserializableObject(vulkanContext, deserializedObject)
{
}

VertexedRenderObject::~VertexedRenderObject() = default;

void VertexedRenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

void VertexedRenderObject::Dispose()
{
	RenderObject::Dispose();
	vertexBuffer->Dispose();
}

