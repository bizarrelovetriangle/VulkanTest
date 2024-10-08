#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

class QueueFamilies;
class Pipeline;
class SwapChain;
class RenderPass;
class Object;
class VulkanContext;

class CommandBuffer
{
public:
	CommandBuffer(VulkanContext& vulkanContext, const vk::Device& device, std::shared_ptr<QueueFamilies> queueFamilies,
		std::shared_ptr<SwapChain> swapChain,
		std::shared_ptr<RenderPass> renderPass);
	void Dispose();

private:
	void CreateCommandPool();
	void CreateCommandBuffer();

public:
	vk::CommandBuffer commandBuffer;
	vk::CommandPool commandPool;

private:
	VulkanContext& vulkanContext;
	const vk::Device& device;
	std::shared_ptr<QueueFamilies> queueFamilies;
	std::shared_ptr<SwapChain> swapChain;
	std::shared_ptr<RenderPass> renderPass;
};