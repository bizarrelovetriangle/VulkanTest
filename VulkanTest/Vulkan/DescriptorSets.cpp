#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "DeviceController.h"
#include "SwapChain.h"
#include "Data/BufferData.h"
#include "Data/ImageData.h"
#include "../Renderers/Interfaces/Renderer.h"

DescriptorSets::DescriptorSets(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout)
	: vulkanContext(vulkanContext), descriptorSetLayout(descriptorSetLayout)
{
	size_t count = vulkanContext.swapChain->frameCount;
	auto& device = vulkanContext.deviceController->device;

	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
	{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, count * 3),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, count)
	};

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, count, descriptorPoolSizes);
	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	std::vector<vk::DescriptorSetLayout> layouts(count, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, layouts);
	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void DescriptorSets::UpdateUniformDescriptor(BufferData& uniform, uint32_t binding)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorBufferInfo descriptorBufferInfo(
			uniform.buffer, 0, uniform.count);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], binding, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo, {});
		vulkanContext.deviceController->device.updateDescriptorSets(writeDescriptorSet, {});
	}
}

void DescriptorSets::UpdateImageDescriptor(ImageData& imageData, uint32_t binding)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorImageInfo descriptorImageInfo(
			imageData.sampler, imageData.imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], binding, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, {}, {});
		vulkanContext.deviceController->device.updateDescriptorSets(writeDescriptorSet, {});
	}
}

void DescriptorSets::Dispose()
{
	vulkanContext.deviceController->device.destroyDescriptorPool(descriptorPool);
}