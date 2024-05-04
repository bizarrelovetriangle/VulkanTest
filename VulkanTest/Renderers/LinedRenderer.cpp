#include "LinedRenderer.h"
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
	vk::VertexInputAttributeDescription colorDescription(
		1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(LinedVertexData, color));
	return { positionDescription, colorDescription };
}

LinedRenderer::LinedRenderer(VulkanContext& vulkanContext, std::vector<Vector4f> colors)
	: VertexedRenderer(vulkanContext), colors(colors)
{
	shared = Shared<LinedRenderer>::getInstance(vulkanContext, true);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*vulkanContext.commonUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 2);
}

void LinedRenderer::UpdateVertexBuffer(const MeshModel& mesh)
{
	if (vertexBuffer) vertexBuffer->Dispose();

	std::vector<LinedVertexData> vertexDatas;
	for (int tri = 0; tri < mesh.triangles.size(); ++tri) {
		auto& triangle = mesh.triangles[tri];

		if (!mesh.triangleBitVector[tri]) continue;

		for (int i = 0; i < 3; ++i) {
			uint32_t org = triangle.vertices[i];
			uint32_t dest = triangle.vertices[(i + 1) % 3];

			for (uint32_t index : { org, dest }) {
				LinedVertexData vertexData;
				vertexData.position = mesh.points[index];
				vertexData.color = !colors.empty() ? colors[index] : propertiesUniform.baseColor;
				vertexDatas.push_back(vertexData);
			}
		}
	}

	vertexBuffer = BufferData::Create<LinedVertexData>(
		vulkanContext, vertexDatas, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
}
