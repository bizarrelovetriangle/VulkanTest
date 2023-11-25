#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <fstream>

class SwapChain;
class VulkanContext;
class RenderObjectShared;

class Pipeline
{
public:
	Pipeline(VulkanContext& vulkanContext, RenderObjectShared& renderObjectShared);
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