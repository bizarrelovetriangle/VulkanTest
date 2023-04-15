#pragma once;
#include <memory>
#include <vulkan/vulkan.hpp>
#include "DeviceMemory.h"

class VulkanContext;

template <class T>
class BufferMemory : public DeviceMemory
{
public:
	BufferMemory(VulkanContext& vulkanContext,
		std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage);
	void FlushData(std::span<std::byte> data) override;
	void Dispose();

public:
	size_t count;
	vk::Buffer buffer;
};
