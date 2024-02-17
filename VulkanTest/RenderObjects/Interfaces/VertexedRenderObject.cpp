#include "VertexedRenderObject.h"
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


VertexedRenderObject::VertexedRenderObject(VulkanContext& vulkanContext)
	: RenderObject(vulkanContext)
{
}

VertexedRenderObject::~VertexedRenderObject() = default;

void VertexedRenderObject::UpdateVertexBuffer(const MeshModel& mesh)
{
	std::vector<VertexData> vertexDatas;

	for (auto& triangle : mesh.triangles) {
		for (int index : triangle.vertices) {
			VertexData vertexData;
			vertexData.position = mesh.points[index];
			vertexData.normal = mesh.TriangleNormal(triangle);
			vertexDatas.push_back(vertexData);
		}
	}

	vertexBuffer = BufferData::Create<VertexData>(
		vulkanContext, vertexDatas, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
}

void VertexedRenderObject::Accept(RenderVisitor& renderVisitor)
{
	renderVisitor.Visit(*this);
}

void VertexedRenderObject::Dispose()
{
	RenderObject::Dispose();
	vertexBuffer->Dispose();
}

