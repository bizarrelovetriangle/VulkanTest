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
	Pipeline(VulkanContext& vulkanContext,
		const vk::Device& device, const vk::RenderPass& renderPass, std::shared_ptr<SwapChain> swapChain);
	void Dispose();
	vk::Viewport CreateViewport();
	vk::Rect2D CreateScissors();

private:
	vk::ShaderModule CreateShaderModule(std::string path);
	void CreateDescriptorSetLayout();

public:
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;

	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::DescriptorSetLayout descriptorSetLayout;

private:
	vk::ShaderModule vertShaderModule;
	vk::ShaderModule fragShaderModule;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	const vk::Device& device;
	const vk::RenderPass& renderPass;
	std::shared_ptr<SwapChain> swapChain;
	VulkanContext& vulkanContext;
	std::unique_ptr<ImageMemory> image;
};