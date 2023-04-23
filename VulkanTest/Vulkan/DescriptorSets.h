#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
class VulkanContext;
class ImageMemory;
class RenderObjectUniform;
template <class T>
class BufferMemory;

class DescriptorSets
{
public:
	DescriptorSets(VulkanContext& vulkanContext, std::vector<vk::DescriptorSetLayoutBinding>& bindings);
	void UpdateUniformDescriptor(BufferMemory<RenderObjectUniform>& uniform, uint32_t binding);
	void UpdateImageDescriptor(ImageMemory& imageMemory, uint32_t binding);
	void Dispose();

	vk::DescriptorSetLayout descriptorSetLayout;
	std::vector<vk::DescriptorSet> descriptorSets;

private:
	VulkanContext& vulkanContext;
	vk::DescriptorPool descriptorPool;
};
