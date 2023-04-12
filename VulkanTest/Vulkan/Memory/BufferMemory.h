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
		const std::vector<T>& data, vk::BufferUsageFlagBits usage);
	void Dispose();

public:
	size_t count;
	vk::Buffer buffer;
};
