#include "PlaneRenderer.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../VulkanContext.h"

PlaneRenderer::PlaneRenderer(VulkanContext& vulkanContext)
	: VertexedRenderer(vulkanContext)
{
	evenPlaneObjectUniformBuffer = BufferData::Create(
		vulkanContext, evenPlaneObjectUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	shared = Shared<PlaneRenderer>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout, DescriptorSetLayoutBinding());
	descriptorSets->UpdateUniformDescriptor(*vulkanContext.commonUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 2);
	descriptorSets->UpdateUniformDescriptor(*evenPlaneObjectUniformBuffer, 3);
}

PlaneRenderer::~PlaneRenderer() = default;

void PlaneRenderer::UpdatePlaneUniformBuffer()
{
	evenPlaneObjectUniformBuffer->FlushData(evenPlaneObjectUniform);
}

void PlaneRenderer::Dispose()
{
	evenPlaneObjectUniformBuffer->Dispose();
	VertexedRenderer::Dispose();
}

std::vector<vk::DescriptorSetLayoutBinding> PlaneRenderer::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}
