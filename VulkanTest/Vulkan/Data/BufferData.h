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
		std::span<std::byte> data, MemoryType memoryType, vk::BufferUsageFlags usage);
	virtual void FlushData(std::span<std::byte> data) override;
	void Dispose();

public:
	size_t count;
	vk::Buffer buffer;
};
