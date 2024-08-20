#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "DeviceController.h"
#include "SwapChain.h"
#include "Data/BufferData.h"
#include "Data/ImageData.h"
#include "../Renderers/Interfaces/Renderer.h"
#include <ranges>

DescriptorSets::DescriptorSets(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout, const std::vector<vk::DescriptorSetLayoutBinding>& descriptorBindings)
	: vulkanContext(vulkanContext), descriptorSetLayout(descriptorSetLayout)
{
	size_t count = vulkanContext.swapChain->frameCount;
	auto& device = vulkanContext.deviceController->device;

	std::unordered_map<vk::DescriptorType, size_t> map;
	for (auto& descriptorBinding : descriptorBindings)
	{
		map[descriptorBinding.descriptorType] += descriptorBinding.descriptorCount;
	}

	auto descriptorPoolSizes = std::vector(std::from_range,
		std::ranges::views::transform(map, [count](auto& p) { return vk::DescriptorPoolSize(p.first, count * p.second); }));
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
			uniform.buffer, 0, uniform.size);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], binding, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo, {});
		vulkanContext.deviceController->device.updateDescriptorSets(writeDescriptorSet, {});
	}
}

void DescriptorSets::UpdateStorageDescriptor(BufferData& storate, uint32_t binding)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorBufferInfo descriptorBufferInfo(
			storate.buffer, 0, storate.size);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], binding, 0, vk::DescriptorType::eStorageBuffer, {}, descriptorBufferInfo, {});
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