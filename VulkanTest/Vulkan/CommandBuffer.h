#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>

#include <vulkan/vulkan.hpp>

class QueueFamilies;
class Pipeline;
class SwapChain;
class RenderPass;
class VertexData;
template <class T>
class VulkanBuffer;
class RenderObjectVertexData;
class RenderObject;

class CommandBuffer
{
public:
	CommandBuffer(const vk::Device& device, std::shared_ptr<QueueFamilies> queueFamilies,
		std::shared_ptr<Pipeline> pipeline, std::shared_ptr<SwapChain> swapChain,
		std::shared_ptr<RenderPass> renderPass);
	void Dispose();
	void RecordCommandBuffer(int imageIndex,
		const std::vector<std::unique_ptr<RenderObject>>& renderObjects);
	void Reset();

private:
	void CreateCommandPool();
	void CreateCommandBuffer();

public:
	vk::CommandBuffer commandBuffer;

private:
	vk::CommandPool commandPool;
	const vk::Device& device;
	std::shared_ptr<QueueFamilies> queueFamilies;
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<SwapChain> swapChain;
	std::shared_ptr<RenderPass> renderPass;
};