#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <fstream>

class SwapChain;
class VulkanContext;
class RendererShared;

class Pipeline
{
public:
	Pipeline(VulkanContext& vulkanContext, RendererShared& RendererShared, bool lined);
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