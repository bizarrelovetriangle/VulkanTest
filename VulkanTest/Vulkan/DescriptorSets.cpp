#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "DeviceController.h"
#include "Memory/ImageMemory.h"

DescriptorSets::DescriptorSets(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout, size_t count)
	: vulkanContext(vulkanContext)
{
	auto& device = vulkanContext.deviceController->device;

	vk::DescriptorPoolSize descriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, count);
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, count, descriptorPoolSize);
	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	std::vector<vk::DescriptorSetLayout> layouts(count, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, layouts);
	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void DescriptorSets::UpdateDescriptor(ImageMemory& imageMemory)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorImageInfo descriptorImageInfo(
			imageMemory.sampler, imageMemory.imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, {}, {});
		vulkanContext.deviceController->device.updateDescriptorSets(writeDescriptorSet, {});
	}
}

void DescriptorSets::Dispose()
{
	vulkanContext.deviceController->device.destroyDescriptorPool(descriptorPool);
}