#include "ColoredRenderer.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../VulkanContext.h"
#include "../CAD/MeshModel.h"

vk::VertexInputBindingDescription ColoredVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(ColoredVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> ColoredVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(ColoredVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(ColoredVertexData, normal));
	vk::VertexInputAttributeDescription colorDescription(
		2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(ColoredVertexData, color));

	return { positionDescription, normalDescription, colorDescription };
}


ColoredRenderer::ColoredRenderer(VulkanContext& vulkanContext, const std::vector<Vector4f>& colors, bool faceColoring)
	: VertexedRenderer(vulkanContext), colors(colors), faceColoring(faceColoring)
{
	shared = Shared<ColoredRenderer>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*vulkanContext.commonUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 2);
}

void ColoredRenderer::UpdateVertexBuffer(const MeshModel& mesh)
{
	std::vector<ColoredVertexData> vertexDatas;

	for (int tri = 0; tri < mesh.triangles.size(); ++tri) {
		auto& triangle = mesh.triangles[tri];

		if (!mesh.triangleBitVector[tri]) continue;

		for (int index : triangle.vertices) {
			ColoredVertexData vertexData;
			vertexData.position = mesh.points[index];
			vertexData.normal = mesh.TriangleNormal(tri);
			vertexData.color = !faceColoring ? colors[index] : colors[tri];
			vertexDatas.push_back(vertexData);
		}
	}

	if (vertexBuffer) vertexBuffer->Dispose();
	vertexBuffer = BufferData::Create<ColoredVertexData>(
		vulkanContext, vertexDatas, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
}
