#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_HEADER_VERSION 239
#define VULKAN_HPP_NO_CONSTRUCTORS
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
	void CreateShaderStages();
	vk::PipelineShaderStageCreateInfo createPipelineShaderStage(
		vk::ShaderStageFlagBits stageFlag, vk::ShaderModule& shaderModule);
	vk::PipelineLayout CreatePipelineLayout();
	vk::PipelineVertexInputStateCreateInfo CreateVertexInputInfo(
		vk::VertexInputBindingDescription binding, std::vector<vk::VertexInputAttributeDescription>& attributes);
	vk::PipelineInputAssemblyStateCreateInfo CreateInputAssemblyInfo();
	vk::PipelineDynamicStateCreateInfo DescribeDinamicStages(std::vector<vk::DynamicState>& dynamicStates);
	vk::PipelineViewportStateCreateInfo CreateViewportState();
	vk::PipelineRasterizationStateCreateInfo CreateRasterizer();
	vk::PipelineMultisampleStateCreateInfo CreateMultisampling();
	vk::PipelineColorBlendAttachmentState CreateColorBlendAttachment();
	vk::PipelineColorBlendStateCreateInfo CreateColorBlendAttachment(
		vk::PipelineColorBlendAttachmentState& colorBlendAttachment);
	vk::ShaderModule CreateShaderModule(std::string path);

public:
	vk::Pipeline graphicsPipeline;

private:
	vk::PipelineLayout pipelineLayout;
	vk::ShaderModule vertShaderModule;
	vk::ShaderModule fragShaderModule;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	const vk::Device& device;
	const vk::RenderPass& renderPass;
	std::shared_ptr<SwapChain> swapChain;
};