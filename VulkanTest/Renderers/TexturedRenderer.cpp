#include "TexturedRenderer.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Vulkan/DeviceController.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../CAD/MeshModel.h"
#include "../VulkanContext.h"
#undef LoadImage;

vk::VertexInputBindingDescription TexturedVertexData::BindingDescription()
{
	return vk::VertexInputBindingDescription(0, sizeof(TexturedVertexData), vk::VertexInputRate::eVertex);
}

std::vector<vk::VertexInputAttributeDescription> TexturedVertexData::AttributeDescriptions()
{
	vk::VertexInputAttributeDescription positionDescription(
		0, 0, vk::Format::eR32G32B32Sfloat, offsetof(TexturedVertexData, position));
	vk::VertexInputAttributeDescription normalDescription(
		1, 0, vk::Format::eR32G32B32Sfloat, offsetof(TexturedVertexData, normal));
	vk::VertexInputAttributeDescription textureCoordDescription(
		2, 0, vk::Format::eR32G32Sfloat, offsetof(TexturedVertexData, textureCoord));

	return { positionDescription, normalDescription, textureCoordDescription };
}


TexturedRenderer::TexturedRenderer(VulkanContext& vulkanContext,
	std::pair<Vector2u, std::vector<std::byte>> textureData, const std::vector<Vector2f>& textureCoords)
	: VertexedRenderer(vulkanContext), textureData(textureData), textureCoords(textureCoords)
{
	auto& [resolution, imageData] = textureData;
	textureBuffer = std::make_unique<ImageData>(
		vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
		MemoryType::Universal);
	textureBuffer->FlushData(imageData);
	textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	shared = Shared<TexturedRenderer>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout, DescriptorSetLayoutBinding());
	descriptorSets->UpdateUniformDescriptor(*vulkanContext.commonUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 2);
	descriptorSets->UpdateImageDescriptor(*textureBuffer, 3);
}

TexturedRenderer::~TexturedRenderer() = default;

void TexturedRenderer::UpdateVertexBuffer(const MeshModel& mesh)
{
	std::vector<TexturedVertexData> vertexDatas;

	for (int tri = 0; tri < mesh.triangles.size(); ++tri) {
		auto& triangle = mesh.triangles[tri];

		if (!mesh.triangleBitVector[tri]) continue;

		for (int index : triangle.vertices) {
			TexturedVertexData vertexData;
			vertexData.position = mesh.points[index];
			vertexData.normal = mesh.TriangleNormal(tri);
			vertexData.textureCoord = textureCoords[index];
			vertexDatas.push_back(vertexData);
		}
	}

	if (vertexBuffer) {
		std::span<TexturedVertexData> vertexSpan = vertexDatas;
		vertexBuffer->FlushData(vertexSpan);
	}
	else {
		vertexBuffer = BufferData::Create<TexturedVertexData>(
			vulkanContext, vertexDatas, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
	}
}

std::vector<vk::DescriptorSetLayoutBinding> TexturedRenderer::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
	};
}

void TexturedRenderer::Dispose()
{
	VertexedRenderer::Dispose();
	textureBuffer->Dispose();
}
