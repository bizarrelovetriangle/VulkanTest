#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include "DeviceMemory/DeviceMemory.h"
#include "../../Utils/Disposable.h"

class VulkanContext;

class BufferData : public Disposable<BufferData>
{
public:
	template <class T>
	static std::unique_ptr<BufferData> Create(VulkanContext& vulkanContext,
		std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage);

	BufferData(VulkanContext& vulkanContext,
		size_t size, MemoryType memoryType, vk::BufferUsageFlags usage);
	template <class T>
	void FlushData(std::span<T> data);
	void DisposeAction();

public:
	size_t count = 0;
	vk::Buffer buffer;

private:
	VulkanContext& vulkanContext;
	DeviceMemory deviceMemory;
};
