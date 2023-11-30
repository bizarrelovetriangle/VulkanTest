#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include "DeviceMemory/DeviceMemory.h"

class VulkanContext;

class BufferData : public DeviceMemory
{
public:
	template <class T>
	static BufferData Create(VulkanContext& vulkanContext,
		std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage);

	BufferData(const BufferData& bufferData);
	BufferData(VulkanContext& vulkanContext,
		size_t size, MemoryType memoryType, vk::BufferUsageFlags usage);
	template <class T>
	void FlushData(std::span<T> data);
	void Dispose();

public:
	size_t count;
	vk::Buffer buffer;
};
