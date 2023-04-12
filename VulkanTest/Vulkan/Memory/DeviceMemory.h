#pragma once;
#include <memory>
#include <vulkan/vulkan.hpp>

class VulkanContext;

enum class MemoryType { Universal, DeviceLocal, HostLocal };

class DeviceMemory
{
public:
	DeviceMemory(VulkanContext& vulkanContext, MemoryType memoryType);
	void AllocateMemory(const vk::MemoryRequirements& memoryRequirements);
	void FlushData(std::span<std::byte> data);
	virtual void StagingFlush(std::span<std::byte> data) = 0;
	void Dispose();

protected:
	uint32_t FindMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	MemoryType memoryType;

protected:
	VulkanContext& vulkanContext;
	vk::DeviceMemory memory;
};
