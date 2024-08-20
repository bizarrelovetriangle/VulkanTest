#include "VertexedRenderer.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/DeviceMemory/DeviceMemory.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"

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


VertexedRenderer::VertexedRenderer(VulkanContext& vulkanContext)
	: Renderer(vulkanContext)
{
}

VertexedRenderer::~VertexedRenderer() = default;

void VertexedRenderer::UpdateVertexBuffer(const MeshModel& mesh)
{
	std::vector<VertexData> vertexDatas;

	for (int tri = 0; tri < mesh.triangles.size(); ++tri) {
		auto& triangle = mesh.triangles[tri];

		if (!mesh.triangleBitVector[tri]) continue;

		for (int index : triangle.vertices) {
			VertexData vertexData;
			vertexData.position = mesh.points[index];
			vertexData.normal = mesh.TriangleNormal(tri);
			vertexDatas.push_back(vertexData);
		}
	}

	if (vertexBuffer) {
		std::span<VertexData> vertexSpan = vertexDatas;
		vertexBuffer->FlushData(vertexSpan);
	}
	else {
		vertexBuffer = BufferData::Create(
			vulkanContext, vertexDatas, MemoryType::Universal, vk::BufferUsageFlagBits::eVertexBuffer);
	}
}

void VertexedRenderer::Accept(RenderVisitor& renderVisitor)
{
	renderVisitor.Visit(*this);
}

void VertexedRenderer::Dispose()
{
	Renderer::Dispose();
	vertexBuffer->Dispose();
}

