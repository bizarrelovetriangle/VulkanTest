#pragma once;
#include <memory>
#include <vulkan/vulkan.hpp>

class VulkanContext;

class DeviceMemory
{
public:
	DeviceMemory(VulkanContext& vulkanContext);
	void AllocateMemory(const vk::MemoryRequirements& memoryRequirements);
	template <class T>
	void FlushData(const std::vector<T>& data);
	void Dispose();

protected:
	uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

protected:
	VulkanContext& vulkanContext;
	vk::DeviceMemory memory;
};
