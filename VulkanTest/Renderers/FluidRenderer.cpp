#include "FluidRenderer.h"
#include "../VulkanContext.h"
#include "../Vulkan/DescriptorSets.h"

FluidRenderer::FluidRenderer(VulkanContext& vulcanContext) : VertexedRenderer(vulcanContext)
{
	shared = Shared<FluidRenderer>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout, DescriptorSetLayoutBinding());
	descriptorSets->UpdateUniformDescriptor(*vulkanContext.commonUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 2);
}

std::vector<vk::DescriptorSetLayoutBinding> FluidRenderer::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}
