#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "Memory/ImageMemory.h"

class SwapChain;
class VulkanContext;

class Pipeline
{
public:
	Pipeline(VulkanContext& vulkanContext, vk::DescriptorSetLayout& descriptorSetLayout);
	void Dispose();

public:
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;

private:
	vk::ShaderModule vertShaderModule;
	vk::ShaderModule fragShaderModule;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	VulkanContext& vulkanContext;
};