#pragma once;
#include <memory>

#define VK_HEADER_VERSION 239
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
class DeviceController;
class ObjVertexData;

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
	vk::Buffer CreateBuffer(size_t size, vk::BufferUsageFlagBits usage);
	uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	vk::DeviceMemory CreateBufferMemory(const vk::MemoryRequirements& memRequirements);

public:
	vk::Buffer buffer;
	size_t count;

private:
	std::shared_ptr<DeviceController> deviceController;
	vk::DeviceMemory bufferMemory;
};
