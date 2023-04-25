#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "DeviceController.h"
#include "SwapChain.h"
#include "Memory/BufferMemory.h"
#include "Memory/ImageMemory.h"
#include "../Primitives/RenderObject.h"

DescriptorSets::DescriptorSets(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout)
	: vulkanContext(vulkanContext), descriptorSetLayout(descriptorSetLayout)
{
	size_t count = vulkanContext.swapChain->frameCount;
	auto& device = vulkanContext.deviceController->device;

	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
	{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, count),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, count)
	};

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, count, descriptorPoolSizes);
	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	std::vector<vk::DescriptorSetLayout> layouts(count, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, layouts);
	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void DescriptorSets::UpdateUniformDescriptor(BufferMemory<RenderObjectUniform>& uniform, uint32_t binding)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorBufferInfo descriptorBufferInfo(
			uniform.buffer, 0, sizeof(RenderObjectUniform));
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], binding, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo, {});
		vulkanContext.deviceController->device.updateDescriptorSets(writeDescriptorSet, {});
	}
}

void DescriptorSets::UpdateImageDescriptor(ImageMemory& imageMemory, uint32_t binding)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorImageInfo descriptorImageInfo(
			imageMemory.sampler, imageMemory.imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], binding, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, {}, {});
		vulkanContext.deviceController->device.updateDescriptorSets(writeDescriptorSet, {});
	}
}

void DescriptorSets::Dispose()
{
	vulkanContext.deviceController->device.destroyDescriptorPool(descriptorPool);
}