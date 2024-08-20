#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
class VulkanContext;
class ImageData;
class RendererUniform;
class BufferData;

class DescriptorSets
{
public:
	DescriptorSets(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout, const std::vector<vk::DescriptorSetLayoutBinding>& descriptorBindings);
	void UpdateUniformDescriptor(BufferData& uniform, uint32_t binding);
	void UpdateStorageDescriptor(BufferData& storate, uint32_t binding);
	void UpdateImageDescriptor(ImageData& imageMemory, uint32_t binding);
	void Dispose();

	vk::DescriptorSetLayout descriptorSetLayout;
	std::vector<vk::DescriptorSet> descriptorSets;

private:
	VulkanContext& vulkanContext;
	vk::DescriptorPool descriptorPool;
};
