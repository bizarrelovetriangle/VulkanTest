#include "TexturedRenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/DeviceController.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/ImageData.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../CAD/MeshModel.h"
#include "../../VulkanContext.h"
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


TexturedRenderObject::TexturedRenderObject(VulkanContext& vulkanContext,
	std::pair<Vector2u, std::vector<std::byte>> textureData, const std::vector<Vector2f>& textureCoords)
	: VertexedRenderObject(vulkanContext), textureData(textureData), textureCoords(textureCoords)
{
	auto& [resolution, imageData] = textureData;
	textureBuffer = std::make_unique<ImageData>(
		vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
		MemoryType::Universal);
	textureBuffer->FlushData(imageData);
	textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	shared = Shared<TexturedRenderObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
	descriptorSets->UpdateImageDescriptor(*textureBuffer, 2);
}

TexturedRenderObject::~TexturedRenderObject() = default;

void TexturedRenderObject::UpdateVertexBuffer(const MeshModel& mesh)
{
	std::vector<TexturedVertexData> vertexDatas;

	for (auto& triangle : mesh.triangles) {
		for (int index : triangle.vertices) {
			TexturedVertexData vertexData;
			vertexData.position = mesh.points[index];
			vertexData.normal = mesh.TriangleNormal(triangle);
			vertexData.textureCoord = textureCoords[index];
			vertexDatas.push_back(vertexData);
		}
	}

	vertexBuffer = BufferData::Create<TexturedVertexData>(
		vulkanContext, vertexDatas, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
}

std::vector<vk::DescriptorSetLayoutBinding> TexturedRenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
	};
}

void TexturedRenderObject::Dispose()
{
	VertexedRenderObject::Dispose();
	textureBuffer->Dispose();
}
