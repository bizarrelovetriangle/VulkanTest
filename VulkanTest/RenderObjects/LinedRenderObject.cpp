#include "LinedRenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../VulkanContext.h"

vk::VertexInputBindingDescription LinedVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(LinedVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> LinedVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(LinedVertexData, position));
	return { positionDescription };
}

LinedRenderObject::LinedRenderObject(VulkanContext& vulkanContext)
	: VertexedRenderObject(vulkanContext)
{
	shared = Shared<LinedRenderObject>::getInstance(vulkanContext, true);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
}

void LinedRenderObject::UpdateVertexBuffer(const MeshModel& mesh)
{
	std::vector<LinedVertexData> vertexDatas;

	for (auto& triangle : mesh.triangles) {
		for (int index : triangle.vertices) {
			LinedVertexData vertexData;
			vertexData.position = mesh.points[index];
			vertexDatas.push_back(vertexData);
		}
	}

	vertexBuffer = BufferData::Create<LinedVertexData>(
		vulkanContext, vertexDatas, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
}
