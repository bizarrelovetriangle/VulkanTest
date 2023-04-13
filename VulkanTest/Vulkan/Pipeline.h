#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <fstream>

class SwapChain;

class Pipeline
{
public:
	Pipeline(const vk::Device& device, const vk::RenderPass& renderPass, std::shared_ptr<SwapChain> swapChain);
	void Dispose();
	vk::Viewport CreateViewport();
	vk::Rect2D CreateScissors();

private:
	vk::ShaderModule CreateShaderModule(std::string path);

public:
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;

private:
	vk::ShaderModule vertShaderModule;
	vk::ShaderModule fragShaderModule;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	const vk::Device& device;
	const vk::RenderPass& renderPass;
	std::shared_ptr<SwapChain> swapChain;
};