#include "DeviceMemory.h"
#include "../DeviceController.h"
#include "../../VulkanContext.h"

DeviceMemory::DeviceMemory(VulkanContext& vulkanContext, MemoryType memoryType)
	: vulkanContext(vulkanContext), memoryType(memoryType)
{
}

void DeviceMemory::AllocateMemory(const vk::MemoryRequirements& memoryRequirements)
{
	vk::MemoryPropertyFlags memoryProperties{};
	if (memoryType == MemoryType::Universal || memoryType == MemoryType::HostLocal)
		memoryProperties |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	if (memoryType == MemoryType::Universal || memoryType == MemoryType::DeviceLocal)
		memoryProperties |= vk::MemoryPropertyFlagBits::eDeviceLocal;

	auto memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties);
	vk::MemoryAllocateInfo allocInfo(memoryRequirements.size, memoryTypeIndex);
	memory = vulkanContext.deviceController->device.allocateMemory(allocInfo);;
}

void DeviceMemory::FlushData(std::span<std::byte> data)
{
	auto dataPointer = vulkanContext.deviceController->device.mapMemory(memory, 0, data.size());
	memcpy(dataPointer, data.data(), data.size());
	vulkanContext.deviceController->device.unmapMemory(memory);
}

uint32_t DeviceMemory::FindMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	auto memProperties = vulkanContext.deviceController->physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void DeviceMemory::Dispose()
{
	vulkanContext.deviceController->device.freeMemory(memory);
}
