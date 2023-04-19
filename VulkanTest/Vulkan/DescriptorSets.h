#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
class VulkanContext;
class ImageMemory;

class DescriptorSets
{
public:
	DescriptorSets(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout, size_t count);
	void UpdateDescriptor(ImageMemory& imageMemory);
	void Dispose();

	std::vector<vk::DescriptorSet> descriptorSets;

private:
	VulkanContext& vulkanContext;
	vk::DescriptorPool descriptorPool;
};
