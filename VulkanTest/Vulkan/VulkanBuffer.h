#pragma once;
#include <memory>

#include <vulkan/vulkan.hpp>
class DeviceController;

template <class T>
class VulkanBuffer
{
public:
	using ValueType = T;

	VulkanBuffer(
		std::shared_ptr<DeviceController> deviceController, const std::vector<T>& data,
		vk::BufferUsageFlagBits usage);
	void FlushData(const std::vector<T>& data);
	void Dispose();

private:
	uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

public:
	vk::Buffer buffer;
	size_t count;

private:
	std::shared_ptr<DeviceController> deviceController;
	vk::DeviceMemory bufferMemory;
};
