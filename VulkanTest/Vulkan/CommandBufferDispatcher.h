#pragma once
#include <vulkan/vulkan.hpp>
#include <functional>
#include <unordered_map>

class VulkanContext;

class CommandBufferDispatcher
{
public:
	CommandBufferDispatcher(VulkanContext& vulkanContext);
	void Invoke(uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> lambda);
	void Dispose();

private:
	VulkanContext& vulkanContext;
	std::unordered_map<uint32_t, vk::CommandPool> commandPoolMap;
};

