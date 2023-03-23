#include "VulkanBuffer.h"
#include "DeviceController.h"
#include "VertexData.h"
#include "../Utils/ObjReader.hpp"

template <class T>
VulkanBuffer<T>::VulkanBuffer(
	std::shared_ptr<DeviceController> deviceController, const std::vector<T>& data,
	vk::BufferUsageFlagBits usage)
	: deviceController(deviceController)
{
	size_t size = sizeof(T) * data.size();
	buffer = CreateBuffer(size, usage);
	auto memRequirements = deviceController->device.getBufferMemoryRequirements(buffer);
	bufferMemory = CreateBufferMemory(memRequirements);
	deviceController->device.bindBufferMemory(buffer, bufferMemory, 0);

	count = data.size();
	FlushData(data);
}

template <class T>
void VulkanBuffer<T>::FlushData(const std::vector<T>& data)
{
	size_t size = sizeof(T) * data.size();
	auto dataPointer = deviceController->device.mapMemory(bufferMemory, 0, size);
	memcpy(dataPointer, data.data(), size);
	deviceController->device.unmapMemory(bufferMemory);
}

template <class T>
vk::Buffer VulkanBuffer<T>::CreateBuffer(size_t size, vk::BufferUsageFlagBits usage)
{
	vk::BufferCreateInfo bufferInfo
	{
		.size = size,
		.usage = usage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	return deviceController->device.createBuffer(bufferInfo);
}

template <class T>
vk::DeviceMemory VulkanBuffer<T>::CreateBufferMemory(const vk::MemoryRequirements& memRequirements)
{
	vk::MemoryAllocateInfo allocInfo
	{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eDeviceLocal |
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent)
	};
	
	return deviceController->device.allocateMemory(allocInfo);;
}

template <class T>
uint32_t VulkanBuffer<T>::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	auto memProperties = deviceController->physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

template <class T>
void VulkanBuffer<T>::Dispose()
{
	deviceController->device.destroyBuffer(buffer);
	deviceController->device.freeMemory(bufferMemory);
}

template VulkanBuffer<RenderObjectVertexData>;
template VulkanBuffer<VertexData>;
template VulkanBuffer<uint16_t>;
